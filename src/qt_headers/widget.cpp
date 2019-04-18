﻿#ifndef SRC_WIDGET
#define SRC_WIDGET

#include "widget.h"
#include <QString>
#include <QWidget>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_released()
{
    QString accout_name = ui->lineEdit->text();
    accout_name.toStdString();
}
#endif // SRC_WIDGET
