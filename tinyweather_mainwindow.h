#ifndef TINYWEATHER_MAINWINDOW_H
#define TINYWEATHER_MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QCategoryAxis>

namespace Ui {
class tinyweather_mainWindow;
}

class tinyweather_mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit tinyweather_mainWindow(QWidget *parent = 0);
    ~tinyweather_mainWindow();

private slots:
    void on_pushButton_clicked();
    void onResult(QNetworkReply* response);


private:
    Ui::tinyweather_mainWindow *ui;
    QNetworkAccessManager m_netManager;
    QtCharts::QChart* m_chart;
    QtCharts::QCategoryAxis * m_axisX;
    QtCharts::QValueAxis * m_axisY;
    QtCharts::QValueAxis * m_tempAxisY;
    QtCharts::QCategoryAxis* m_axisMarker;
};

#endif // TINYWEATHER_MAINWINDOW_H
