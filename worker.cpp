#include "worker.h"
#include "whisperhelper.h"
#include <whisper/whisper.h>
#include <QCoreApplication>
#include <QFile>
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
Worker::Worker(QObject *parent):QThread(parent)
{

}

void Worker::setParams(QString filepath, QString modelpath, QString language,QString outputtype,bool translate,bool fast,int processors,int threads)
{
    this->filepath = filepath;
    this->modelpath = modelpath;
    this->language = language;
    this->outputtype = outputtype;
    this->translate = translate;
    this->fast = fast;
    this->processors = processors;
    this->threads = threads;
}

void Worker::run()
{
    emit report(QString("Format Changing..."));
    QStringList soxargs;
    QString tempname = QCoreApplication::applicationDirPath()+"/temp.wav";
    soxargs<<"-b"<<"16"<<"-c"<<"1"<<"-r"<<"16k"<<filepath.replace("/","\\")<<tempname.replace("/","\\");
    QProcess *psox = new QProcess(nullptr);
    psox->start(QCoreApplication::applicationDirPath()+"/sox.exe",soxargs);
    if(!psox->waitForFinished()){
        emit raise(QString("Sox procress error.Maybe not supported file format."));
        return;
    }
    emit report(QString("Init Model..."));
    QString modelstr = QCoreApplication::applicationDirPath()+"/model/"+this->modelpath;
    whisper_context * ctx = whisper_init(modelstr.toStdString().c_str());
    if(ctx == nullptr){
        emit raise(QString("Could not load model at %1.").arg(this->modelpath));
        return;
    }
    std::vector<float> pcmf32; // mono-channel F32 PCM
    std::vector<std::vector<float>> pcmf32s; // stereo-channel F32 PCM
    drwav wav;
    emit report(QString("Init Wav Data..."));
    if (drwav_init_file(&wav, tempname.toStdString().c_str(), NULL) == false) {
        emit raise(QString("Read output from Sox error."));
        return;
    }
    const uint64_t n = wav.totalPCMFrameCount;
    std::vector<int16_t> pcm16;
    pcm16.resize(n*wav.channels);
    drwav_read_pcm_frames_s16(&wav, n, pcm16.data());
    drwav_uninit(&wav);
    pcmf32.resize(n);
    if (wav.channels == 1) {
        for (uint64_t i = 0; i < n; i++) {
            pcmf32[i] = float(pcm16[i])/32768.0f;
        }
    } else {
        for (uint64_t i = 0; i < n; i++) {
            pcmf32[i] = float(pcm16[2*i] + pcm16[2*i + 1])/65536.0f;
        }
    }
    if (!whisper_is_multilingual(ctx)) {
        if (this->language != "en" || this->translate) {
            this->language = "en";
            this->translate = false;
            emit report(QString("model is not multilingual, ignoring language and translation options"));
        }
    }
    emit report(QString("Processing..."));
    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.print_realtime   = false;
    wparams.print_progress   = false;
    wparams.print_timestamps = false;
    wparams.print_special    = false;
    wparams.translate        = this->translate;
    wparams.language         = this->language.toStdString().c_str();
    wparams.n_threads        = this->threads;
    wparams.max_len          = 60;
    wparams.speed_up         = this->fast;
    wparams.encoder_begin_callback = nullptr;
    if (whisper_full_parallel(ctx, wparams, pcmf32.data(), pcmf32.size(), this->processors) != 0) {
        emit raise(QString("Unable to process audio file."));
        return;
    }
    std::string output;
    if(this->outputtype==QString("txt")){
        output = output_txt(ctx);
    }
    else if (this->outputtype==QString("vtt")){
        output = output_vtt(ctx);
    }
    else if (this->outputtype==QString("srt")){
        output = output_srt(ctx);
    }
    emit report(QString("Cleaning..."));
    QFile ftemp(tempname);
    ftemp.remove();
    emit done(QString::fromStdString(output));
    whisper_free(ctx);
    delete psox;
    return;
}

