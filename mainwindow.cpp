#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lang.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QClipboard>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMinMaxButtonsHint|Qt::WindowMinimizeButtonHint);
    this->setFixedSize(this->width(),this->height());
    this->wok = new Worker(this);
    QDir modeldir = QDir(QCoreApplication::applicationDirPath()+"/model");
    if(!modeldir.exists()){
        modeldir.mkdir(QCoreApplication::applicationDirPath()+"/model");
        QMessageBox::warning(this,QString("Warning:"),QString("No Model Founded."));
        exit(1);
    }
    modeldir.setFilter(QDir::Files);
    QFileInfoList modellist = modeldir.entryInfoList();
    for(QFileInfo finfo : modellist){
        if(finfo.isFile() && finfo.fileName().endsWith(QString(".bin"))){
            ui->modeCombo->addItem(finfo.fileName());
        }
    }
    if(ui->modeCombo->count()==0){
        QMessageBox::warning(this,QString("Warning:"),QString("No Model Founded."));
        exit(1);
    }
    ui->modeCombo->setCurrentIndex(0);
    ui->typeCombo->addItem(QString("txt"));
    ui->typeCombo->addItem(QString("vtt"));
    ui->typeCombo->addItem(QString("srt"));
    ui->typeCombo->setCurrentIndex(0);
    for(auto i : g_lang){
        ui->LanguageCombo->addItem(QString(i.second.c_str()));
    }

    ui->LanguageCombo->setCurrentText(QString("english"));
    connect(wok,SIGNAL(raise(QString)),this,SLOT(recerr(QString)));
    connect(wok,SIGNAL(report(QString)),this,SLOT(reclog(QString)));
    connect(wok,SIGNAL(done(QString)),this,SLOT(recresult(QString)));
    ui->spinBox_2->setMaximum(QThread::idealThreadCount());
    ui->spinBox_2->setValue(QThread::idealThreadCount()/2);
}

MainWindow::~MainWindow()
{
    if(wok->isRunning()){
        wok->terminate();
    }
    delete wok;
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,QString("Select Audio:"),QString(),
                                                    QString("Audio File(*.*)"));
    if(!(filename.isNull()||filename.isEmpty()))
    {
        ui->lineEdit->setText(filename);
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    QString m = ui->textBrowser->toPlainText();
    if(m.isEmpty()||m.isNull()){
        QMessageBox::warning(this,QString("Warning"),QString("Nothing to save."));
        return;
    }
    QString type = ui->typeCombo->currentText();
    QString savefile = QFileDialog::getSaveFileName(this,QString("Save as ")+type,QString(),
                                                    type+QString(" file(*.")+type+QString(")"));
    if(!(savefile.isNull()||savefile.isEmpty()))
    {
        QFile file(savefile);
        if(file.open(QFile::WriteOnly|QFile::Text))
        {
            QTextStream in(&file);
            in<<m;
        }
        else {
            QMessageBox::critical(this,QString("Error"),QString("Unable to write file."));
        }
        file.close();
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    QString m = ui->textBrowser->toPlainText();
    if(m.isEmpty()||m.isNull()){
        QMessageBox::warning(this,QString("Warning"),QString("Nothing to copy."));
        return;
    }
    QGuiApplication::clipboard()->setText(m);
}

void MainWindow::on_pushButton_4_clicked()
{
    QString file = ui->lineEdit->text();
    QString type = ui->typeCombo->currentText();
    QString model = ui->modeCombo->currentText();
    auto langfull = ui->LanguageCombo->currentText().toStdString();
    QString lang;
    for (auto u : g_lang){
        if(langfull.compare(u.second)==0){
            lang = QString(u.first.c_str());
            break;
        }
    }
    bool trans = ui->checkBox->isChecked();
    bool fast = ui->checkBox_2->isChecked();
    int proc = ui->spinBox->value();
    int thr = ui->spinBox_2->value();
    if(file.isNull()||file.isEmpty()){
        QMessageBox::warning(this,QString("Warning"),QString("No input File."));
        return;
    }
    if(type.isNull()||type.isEmpty()){
        QMessageBox::warning(this,QString("Warning"),QString("Type Error."));
        return;
    }
    if(model.isNull()||model.isEmpty()){
        QMessageBox::warning(this,QString("Warning"),QString("Model Error."));
        return;
    }
    if(lang.isNull()||lang.isEmpty()){
        QMessageBox::warning(this,QString("Warning"),QString("Language Error."));
        return;
    }
    wok->setParams(file,model,lang,type,trans,fast,proc,thr);
    wok->start();
}

void MainWindow::recerr(QString ms)
{
    reclog(QString("Error: ")+ms);
    QMessageBox::critical(this,QString("Error"),ms);
}

void MainWindow::recresult(QString ms)
{
    ui->textBrowser->setText(ms);
    QMessageBox::information(this,QString("Done"),QString("Finish Process."));
    reclog(QString("Finish."));
}

void MainWindow::reclog(QString ms)
{
    QString log = ui->textBrowser_2->toPlainText();
    QStringList loglist = log.split(QString("\n"));
    QString newlog;
    if(loglist.length()>19){
        for (int i = 0;i<19;i++){
            newlog = loglist.at(loglist.length()-i-1)+ "\r\n" + newlog;
        }
        newlog += ms;
    }
    else {
        newlog = log + "\r\n" + ms;
    }
    ui->textBrowser_2->setText(newlog);
}

