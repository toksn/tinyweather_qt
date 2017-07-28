#include <QUrl>
#include <QtCharts/QLineSeries>
#include <QtNetwork>

#include "tinyweather_mainwindow.h"
#include "ui_tinyweather_mainwindow.h"

tinyweather_mainWindow::tinyweather_mainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tinyweather_mainWindow)
{
    ui->setupUi(this);
    connect(&m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));

    m_chart = new QtCharts::QChart();
    m_chart->legend()->hide();

    m_axisY = new QtCharts::QValueAxis();
    m_axisY->setGridLineVisible(true);
    m_axisY->setRange(0, 100);
    m_axisY->setTickCount(5);
    m_axisY->setLabelFormat("%i");
    m_axisY->setLabelsColor(QColor("blue"));
    m_chart->addAxis(m_axisY, Qt::AlignLeft);


    /*QtCharts::QCategoryAxis* axis5050 = new QtCharts::QCategoryAxis();
    axis5050->setRange(0,10);
    axis5050->setStartValue(5);
    axis5050->append("", 10);
    axis5050->setShadesPen(Qt::NoPen);
    axis5050->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
    axis5050->setShadesVisible(true);
    axis5050->setReverse(true);
    m_chart->addAxis(axis5050, Qt::AlignRight);*/


    QtCharts::QChartView* chartView = new QtCharts::QChartView(m_chart);
    ui->verticalLayout->addWidget(chartView);
}

tinyweather_mainWindow::~tinyweather_mainWindow()
{
    delete ui;
}

void tinyweather_mainWindow::on_pushButton_clicked()
{
    QString key = "01f9bf4a3d561a37";
    QString URL = "http://api.wunderground.com/api/" + key + "/conditions/hourly/lang:DL/q/Germany/Leipzig.json";
    QUrl serviceURL;
    serviceURL.setUrl(URL);
    QNetworkRequest request(serviceURL);
    request.setRawHeader("Content-Type", "application/json");

    m_netManager.get(request);
    //connect(response, &QNetworkReply::finished, this, &tinyweather_mainWindow::onResponseFinished);
    //connect(response, &QIODevice::readyRead, this, &tinyweather_mainWindow::onResponseFinished);
}


void tinyweather_mainWindow::onResult(QNetworkReply* response)
{
    QString textv;
    QByteArray json_data;

    if(response)
        json_data = response->readAll();

    if (json_data.length() > 0)
    {
        QJsonDocument weather_data = QJsonDocument::fromJson(json_data);
        QJsonObject curObs = weather_data.object()["current_observation"].toObject();

        QString city = curObs["display_location"].toObject()["city"].toString();

        float temp_c    = curObs["temp_c"].toDouble();
        float wind      = curObs["wind_kph"].toDouble();

        textv = QString("Aktuelle Temperatur in %1: %2 °C").arg(city).arg(temp_c);
        textv += "\n" + curObs["weather"].toString();
        textv += "\n\nWind: " + QString("%1").arg(wind) + "km/h";
        if(wind > 0.0)
            textv += " aus Richtung " + curObs["wind_dir"].toString();
        textv += "\nLuftfeuchtigkeit: " + curObs["relative_humidity"].toString();


        // recreate chart information
        if(m_chart)
        {
            m_chart->removeAllSeries();

            QtCharts::QLineSeries* pts_data = new QtCharts::QLineSeries();
            QtCharts::QLineSeries* temp_data = new QtCharts::QLineSeries();
            //temp_data->setColor(QColor("orange"));
            //pts_data->setColor(QColor("blue"));

            QPen pen1, pen2;
            pen1.setWidth(3);
            pen2.setWidth(3);
            pen1.setColor(QColor("orange"));
            pen2.setColor(QColor("blue"));
            pts_data->setPen(pen2);
            temp_data->setPen(pen1);
            //QtCharts::QBarSeries* bar = new QtCharts::QBarSeries();
            QJsonArray hourly = weather_data.object()["hourly_forecast"].toArray();
            int i = 0;

            //QDateTime hourCorrect;
            //hourCorrect.setDate(QDate::currentDate());
            double actualHour = 0;

            if(m_axisMarker)
                m_chart->removeAxis(m_axisMarker);

            m_axisMarker = new QtCharts::QCategoryAxis();
            m_axisMarker->setLabelsPosition(QtCharts::QCategoryAxis::AxisLabelsPositionOnValue);

            double temp_min = 0, temp_max = 0;
            for(auto it = hourly.begin(); it != hourly.end() && i < 25; ++it)
            {
                QJsonObject curHour = (*it).toObject();
                QString hour = curHour["FCTTIME"].toObject()["hour"].toString();
                QString rain = curHour["pop"].toString();
                QString temp = curHour["temp"].toObject()["metric"].toString();

                //if(i==0)
                //    hourCorrect.addSecs(hour.toDouble()*3600);
                //else
                //    hourCorrect.addSecs(3600);
                double dTemp = temp.toDouble();
                if(i==0)
                {
                    temp_min = temp_max = dTemp;
                    actualHour=hour.toDouble();
                }
                else
                {
                    if(dTemp<temp_min) temp_min = dTemp;
                    else if(dTemp>temp_max) temp_max = dTemp;

                    actualHour++;
                }

                //pts_data->append(hour.toDouble(), rain.toDouble());
                pts_data->append(actualHour, rain.toDouble());
                temp_data->append(actualHour, dTemp);
                if(i%4 == 0)
                    m_axisMarker->append(QString("%1").arg(((int)actualHour)%24), actualHour);


                //QtCharts::QBarSet* barset = new QtCharts::QBarSet(QString("%1").arg(hour));
                //barset->append(rain);
                //bar->append(barset);
                i++;
            }

            pts_data->setName("Niederschlag");
            temp_data->setName("Temperatur");
            m_chart->addSeries(pts_data);
            m_chart->addSeries(temp_data);
            m_chart->legend()->show();
            //m_chart->createDefaultAxes();

            // recreate temp y axis
            if(m_tempAxisY)
                m_chart->removeAxis(m_tempAxisY);

            m_tempAxisY= new QtCharts::QValueAxis();
            double curRange = temp_max - temp_min;
            if(curRange>20)
            {
                temp_min-= 2.0;
                temp_max+= 2.0;
            }
            else
            {
                curRange=(20.0-curRange)*0.5;
                temp_min-=curRange;
                temp_max+=curRange;

                temp_min = floor(temp_min);
                temp_max = floor(temp_max);
            }
            m_tempAxisY->setRange(temp_min,temp_max);
            //m_tempAxisY->applyNiceNumbers();
            m_tempAxisY->setTickCount(5);
            m_tempAxisY->setLabelFormat(QString("%.1f").toUtf8());
            //m_tempAxisY->setLabelFormat(QString("%i").toUtf8());
            m_tempAxisY->setLabelsColor(QColor("orange"));
            m_chart->addAxis(m_tempAxisY, Qt::AlignRight);
            temp_data->attachAxis(m_tempAxisY);



             //recreate x axis
            if(m_axisX)
                m_chart->removeAxis(m_axisX);

            double start_marker1 = 7.5;
            double end_marker1 = 9.5;
            double start_marker2 = 17;
            double end_marker2 = 19;
            m_axisX = new QtCharts::QCategoryAxis();

            //m_axisX->setStartValue(start_marker1);
            m_axisX->append("    ", start_marker1);
            m_axisX->append("A", end_marker1);

            m_axisX->append("", start_marker2);
            m_axisX->append("B", end_marker2);

            m_axisX->append("  ", start_marker1+24);
            m_axisX->append("C", end_marker1+24);
            m_axisX->append("   ", start_marker2+24);
            m_axisX->append("D", end_marker2+24);
            m_axisX->setShadesPen(Qt::NoPen);
            m_axisX->setShadesBrush(QBrush(QColor(0x99, 0xcc, 0xcc, 0x55)));
            m_axisX->setShadesVisible(true);

            m_chart->addAxis(m_axisX, Qt::AlignTop);



            m_chart->addAxis(m_axisMarker, Qt::AlignBottom);


            pts_data->attachAxis(m_axisX);
            pts_data->attachAxis(m_axisY);
            pts_data->attachAxis(m_axisMarker);




            //m_chart->addSeries(bar);
            //m_chart->setTheme(QtCharts::QChart::ChartTheme::ChartThemeQt);
        }
    }
    else
    {
        textv = "Current temperature data could not be received.";
    }

    ui->txt_label->setText(textv);
}
