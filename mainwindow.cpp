#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>

MainWindow *handle;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->count = 50;
    this->time = 0;
    this->setWindowTitle("EE513 Assignment 2");
    this->ui->customPlot->addGraph();
    this->ui->customPlot->yAxis->setLabel("Pitch (degrees)");
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot->xAxis->setTicker(timeTicker);
    this->ui->customPlot->yAxis->setRange(-180,180);
    this->ui->customPlot->replot();
    ::handle = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    // For more help on real-time plots, see: http://www.qcustomplot.com/index.php/demos/realtimedatademo
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    ui->customPlot->graph(0)->addData(key,count);
    ui->customPlot->graph(0)->rescaleKeyAxis(true);
    ui->customPlot->replot();
    QString text = QString("Value added is %1").arg(this->count);
    ui->outputEdit->setText(text);
}

void MainWindow::on_downButton_clicked()
{
    this->count-=10;
    this->update();
}

void MainWindow::on_upButton_clicked()
{
    this->count+=10;
    this->update();
}

void MainWindow::on_connectButton_clicked()
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        ui->outputText->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->outputText->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->outputText->appendPlainText(QString("Subscribing to topic " TOPIC " for client " CLIENTID));
    int x = MQTTClient_subscribe(client, TOPIC, QOS);
    ui->outputText->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    handle->ui->outputText->appendPlainText("Message delivery confirmed");
    handle->deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)context; (void)topicLen;
    handle->ui->outputText->appendPlainText(QString("Message arrived (topic is %1)").arg(topicName));
    char* payloadptr = (char*) message->payload;
    handle->ui->outputText->appendPlainText(QString("Message payload length is %1").arg(message->payloadlen));
    QString msg;
    msg.sprintf("[%s]",payloadptr);
    qDebug() << "msg is: " << payloadptr << endl;
    handle->ui->outputText->appendPlainText(msg);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    (void)context; (void)*cause;
    handle->ui->outputText->appendPlainText("Connection Lost");
}

void MainWindow::on_disconnectButton_clicked()
{
    handle->ui->outputText->appendPlainText("Disconnecting from the broker");
    MQTTClient_disconnect(client, 10000);
    //MQTTClient_destroy(&client);
}
