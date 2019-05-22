﻿#ifndef SRC_WIDGET
#define SRC_WIDGET

#pragma execution_character_set("utf-8")

#include <string>
#include <sstream>

#include "widget.h"
#include <QString>
#include <QWidget>
#include <QMessageBox>
#include <QTableWidget>
#include <QStringList>
#include <QTableWidgetItem>
#include <QPixmap>
#include <QTextCodec>
#include <QComboBox>

#include "time_table_widget_item.h"

Widget::Widget(QWidget *parent) : QWidget(parent),
                                  ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->Path_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);
    //qDebug() << &idmap_widget << endl;
    ui->MapLabel->initializMyLabel(&idmap_widget);
    simulator.initialize(ui->ClockSimulate, ui->MapSimulate);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_LogInButton_released() //登陆
{
    //QString temp;
    //temp.fromStdString(idmap_widget.GetCityStr(13));
    //qDebug() << temp;
    QString account_name = ui->lineEdit->text();

    if (!account_name.size())
    {
        QMessageBox::warning(this, "Warning!", "输入为空", QMessageBox::Ok);
        return;
    }
    else if (!IsValidName(account_name.toStdString()))
    {
        QMessageBox::warning(this, "Warning!", "非法的用户名,请重新输入", QMessageBox::Ok);
        return;
    }

    int account_check = AccountCheck(account_name.toStdString());

    if (account_check != -1)
    {
        traveller_widget.LoadData(account_check, citygraph_widget);
        traveller_widget.set_id(account_name.toStdString());
        ui->stackedWidget->setCurrentWidget(ui->MainPage);
    }
    else
    {
        QMessageBox::StandardButton sB = QMessageBox::question(this, "", QString("该账号不存在，是否注册:") + account_name, QMessageBox::Yes | QMessageBox::No);

        if (sB == QMessageBox::Yes)
        {
            AddAccount(account_name.toStdString());
            traveller_widget.set_id(account_name.toStdString());
            ui->stackedWidget->setCurrentWidget(ui->MainPage);
        }
    }
}

void Widget::on_SignUpButton_released() //注册
{
    QString account_name = ui->lineEdit->text();

    if (!account_name.size())
    {
        QMessageBox::warning(this, "Warning!", "输入为空", QMessageBox::Ok);
        return;
    }
    else if (!IsValidName(account_name.toStdString()))
    {
        QMessageBox::warning(this, "Warning!", "非法的用户名,请重新输入", QMessageBox::Ok);
        return;
    }

    int account_check = AccountCheck(account_name.toStdString());

    if (account_check != -1)
    {
        QMessageBox::warning(this, "Warning!", "该账号已被注册，请重新输入", QMessageBox::Ok);
        return;
    }
    else
    {
        AddAccount(account_name.toStdString()); //?
        traveller_widget.set_id(account_name.toStdString());
        ui->stackedWidget->setCurrentWidget(ui->MainPage);
    }
}

void Widget::on_OrderPageButton_released() // 预定行程
{
    ui->MapLabel->setOriginPixmap();
    ui->stackedWidget->setCurrentWidget(ui->OrderPage);
    if(ui->strategy_comboBox->currentIndex() != 2)
        ui->limit_time_widget->hide();
}

void Widget::on_strategy_comboBox_currentIndexChanged(int index)
{
    if(index == 2)
        ui->limit_time_widget->show();
    else ui->limit_time_widget->hide();
}

void Widget::on_StatePageButton_released() // 状态查询
{
    ui->stackedWidget->setCurrentWidget(ui->StatePage);
    ui->stateTableWidget->clearContents();
    ui->stateTableWidget->setSortingEnabled(false);

    const QString empty_plan_qstr("无");
    std::string temp_str;
    auto plan = traveller_widget.get_plan();
    auto path = traveller_widget.get_path();
    auto position = traveller_widget.get_position();

    ui->state_user_name_content->setText(QString::fromStdString(traveller_widget.get_ID()));
    if (position == -2)
    {
        ui->state_start_city_content->setText(empty_plan_qstr);
        ui->state_target_city_content->setText(empty_plan_qstr);
        ui->state_pass_content->setText(empty_plan_qstr);
        ui->state_time_content->setText(empty_plan_qstr);
        ui->state_price_content->setText(empty_plan_qstr);
        ui->state_position_content->setText(empty_plan_qstr);
        return;
    }
    else
    {
        ui->state_start_city_content->setText(QString::fromStdString(idmap_widget.GetCityStr(plan.front())));
        ui->state_target_city_content->setText(QString::fromStdString(idmap_widget.GetCityStr(plan.back())));

        if (plan.size() > 2)
        {
            temp_str.clear();
            for (int i = 1; i < plan.size() - 1; i++)
            {
                temp_str += idmap_widget.GetCityStr(plan.at(i)) + " ";
            }
            ui->state_pass_content->setText(QString::fromStdString(temp_str));
        }
        else
        {
            ui->state_pass_content->setText(empty_plan_qstr);
        }
        ui->state_time_content->setNum(path.GetTotalTime().GetLength());
        ui->state_price_content->setNum(path.GetTotalPrice());

        if (position == -1)
        {
            ui->state_position_content->setText(empty_plan_qstr);
        }
        else
        {
            ui->state_position_content->setText(QString::fromStdString(idmap_widget.GetCityStr(path.GetNode(position).former_city)));
        }
    }

    ui->stateTableWidget->setRowCount(path.GetLen());
    for (int i = 0; i < path.GetLen(); i++)
    {
        auto &node = path.GetNode(i);
        UpdateTable(ui->stateTableWidget, i, node.former_city, node.current_city, node.kth_way);
    }

    ui->stateTableWidget->setSortingEnabled(true);
}

void Widget::on_QueryPathPageButton_released() // 路线查询
{
    ui->stackedWidget->setCurrentWidget(ui->QueryPage);
    ui->Path_tableWidget->clearContents();
    ui->Path_tableWidget->setRowCount(0);
}

void Widget::on_SimulationPageButton_released() // 开始模拟
{
    ui->stackedWidget->setCurrentWidget(ui->SimulatePage);
    simulator.start();
}

void Widget::on_StatePageToMenuButton_released() // 从状态页面回到主菜单
{
    ui->stackedWidget->setCurrentWidget(ui->MainPage);
}

void Widget::on_OrderPageToMenuButton_released() // 从预定页面返回到主菜单
{
    ui->stackedWidget->setCurrentWidget(ui->MainPage);
}

void Widget::on_QueryPageToMenuButton_released() // 从查询路线页面返回到主菜单
{
    ui->stackedWidget->setCurrentWidget(ui->MainPage);
}

void Widget::on_SimuToMenuButton_released()// 从模拟返回到主菜单
{
    ui->stackedWidget->setCurrentWidget(ui->MainPage);
}


void Widget::UpdateTable(QTableWidget *table, int row, City_id start_city, City_id target_city, int k)
{
    QString start_city_qstr, target_city_qstr, transport_type_qstr, start_time_qstr, end_time_qstr, price_qstr;
    std::string start_time_str, end_time_str;
    std::stringstream ss;

    start_city_qstr = QString::fromStdString(idmap_widget.GetCityStr(start_city));
    target_city_qstr = QString::fromStdString(idmap_widget.GetCityStr(target_city));

    Route r = citygraph_widget.GetRoute(start_city, target_city, k);

    transport_type_qstr = QString::fromStdString(idmap_widget.GetTransStr(r.transport_type));

    ss.str(RouteShow(r.start_time, r.end_time));
    ss.clear();
    ss >> start_time_str >> end_time_str;
    start_time_qstr = QString::fromStdString(start_time_str);
    end_time_qstr = QString::fromStdString(end_time_str);

    price_qstr = QString::fromStdString(std::to_string(r.price));

    // qDebug() << start_city_qstr << " " << target_city_qstr << " " << transport_type_qstr << " " << start_time_qstr << " " << end_time_qstr << " " << price_qstr << endl;

    QTableWidgetItem *id_cell = new QTableWidgetItem;
    id_cell->setData(Qt::DisplayRole, row + 1);

    QTableWidgetItem *start_city_cell = new QTableWidgetItem(start_city_qstr),
                     *target_city_cell = new QTableWidgetItem(target_city_qstr),
                     *transport_type_cell = new QTableWidgetItem(transport_type_qstr),
                     *start_time_cell = new TimeTableWidgetItem(start_time_qstr),
                     *end_time_cell = new TimeTableWidgetItem(end_time_qstr),
                     *price_cell = new QTableWidgetItem(price_qstr);

    table->setItem(row, 0, id_cell);
    table->setItem(row, 1, start_city_cell);
    table->setItem(row, 2, target_city_cell);
    table->setItem(row, 3, transport_type_cell);
    table->setItem(row, 4, start_time_cell);
    table->setItem(row, 5, end_time_cell);
    table->setItem(row, 6, price_cell);
}

void Widget::on_QueryPathButton_released()
{
    City_id start_city = ui->origin_comboBox->currentIndex();
    City_id target_city = ui->target_comboBox->currentIndex();
    ui->Path_tableWidget->clearContents();
    ui->Path_tableWidget->setRowCount(0);
    ui->Path_tableWidget->setSortingEnabled(false);

    if (start_city == target_city)
    {
        QMessageBox::warning(this, "Warning!", "始发地不能与目的地一样", QMessageBox::Ok);
        return;
    }

    int size = citygraph_widget.Getsize(start_city, target_city);
    if (!size)
    {
        QMessageBox::warning(this, "Warning!", "两城市间无路线", QMessageBox::Ok);
    }
    else
    {
        auto path_table = ui->Path_tableWidget;
        path_table->setRowCount(size);

        std::string temp;
        for (int i = 0; i < size; i++)
        {
            UpdateTable(path_table, i, start_city, target_city, i);
        }
    }
    ui->Path_tableWidget->setSortingEnabled(true);
}

void Widget::on_OrderPathButton_released()
{
    if(!ui->MapLabel->has_origin())
    {
        QMessageBox::warning(this, "Warning!", "未设置始发地", QMessageBox::Ok);
    }
    else if(!ui->MapLabel->has_destination())
    {
        QMessageBox::warning(this, "Warning!", "未设置目的地", QMessageBox::Ok);
    }
    else
    {
        Strategy s = static_cast<Strategy>(ui->strategy_comboBox->currentIndex());
        Time init_time(1,ui->init_time_comboBox->currentIndex());
        if(s == 2)
        {
            Time limit_time(ui->limit_day_spinbox->value(), ui->limit_hour_comboBox->currentIndex());
            traveller_widget.set_plan(ui->MapLabel->getplan());
            Path p = traveller_widget.SchedulePath(citygraph_widget, s, init_time, limit_time);
            if(!p.GetLen())
            {
                QMessageBox::warning(this, "Warning!", "未找到符合要求路径", QMessageBox::Ok);
                return;
            }
            traveller_widget.set_path(p);
        }
        else
        {
            traveller_widget.set_plan(ui->MapLabel->getplan());
            Path p = traveller_widget.SchedulePath(citygraph_widget, s, init_time);
            if(!p.GetLen())
            {
                QMessageBox::warning(this, "Warning!", "未找到符合要求路径", QMessageBox::Ok);
                return;
            }
            traveller_widget.set_path(p);
        }
        QMessageBox::information(this, "Success!", "已预定行程" ,QMessageBox::Ok);
    }
}


#endif // SRC_WIDGET
