#ifndef SRC_MYLABEL
#define SRC_WYLABEL

#pragma execution_character_set("utf-8")

#include "mylabel.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPixmap>
#include <QString>
#include <QMessageBox>
#include <cmath>
#include <QResizeEvent>
#include <deque>
#include <windows.h>

static const double r_city = 10; //半径

MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
    this->setScaledContents(true);
    // this->setScaledContents(false);
    // this->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

void MyLabel::setOriginPixmap()
{
    if (!hasOriginPixmap())
        origin_pixmap = *pixmap();
}

bool MyLabel::hasOriginPixmap()
{
    return !origin_pixmap.isNull();
}

void MyLabel::mouseMoveEvent(QMouseEvent *ev)
{
    mouse_x_ = ev->pos().x();
    mouse_y_ = ev->pos().y();
    int i = judge_mouse_pos();
    if (i != -1)
    {
        Sleep(400);
        setToolTip(QString::fromStdString(idmap->getCityStr(i)));
    }
    else
    {
        setToolTip(NULL);
    }
}

void MyLabel::mousePressEvent(QMouseEvent *ev)
{
    //qDebug() << ev->x() << ev->y();
    if (ev->button() == Qt::LeftButton)
    {
        //qDebug() << mark_origin->height() << mark_origin->width();
        //qDebug() << ev->x() << ev->y();
        mouse_x_ = ev->x();
        mouse_y_ = ev->y() + mark_origin->height() / 4;
        int i = judge_mouse_pos();
        if (i != -1 && has_mark_transfer[i])
        {
            has_mark_transfer[i] = false;
            mark_transfer[i]->hide();
            delete_transfer_city(i);
            QString temp = "途经城市：";
            for(auto city : transfer_city)
            {
                temp += QString::fromStdString(idmap->getCityStr(city)) + " ";
            }
            transfercityBrowser_->setText(temp);
        }
    }
    else if (ev->button() == Qt::RightButton)
    {
        mouse_x_ = ev->x();
        mouse_y_ = ev->y();
        //QString temp;
        int i = judge_mouse_pos();
        //qDebug() << i;
        //if(i!=-1)
        //temp = QString::fromStdString(idmap->GetCityStr(i));
        //qDebug() << temp;
        if (i != -1 && where_mark_destination != i && where_mark_origin != i && !has_mark_transfer[i])
        {
            current_set_city = i;
            rightbutton_menu->popup(ev->globalPos());
        }
    }
}

void MyLabel::origin_action_triggered()
{
    double x = mouse_x_, y = mouse_y_;
    double city_x = city_pos_[current_set_city].first, city_y = city_pos_[current_set_city].second;
    double w_ratio = static_cast<double>(current_qsize_.width()) / static_cast<double>(origin_qsize_.width());
    double h_ratio = static_cast<double>(current_qsize_.height()) / static_cast<double>(origin_qsize_.height());
    city_x *= w_ratio;
    city_y *= h_ratio;

    mark_origin->move(city_x - 33, city_y - 55);
    rect_mark_origin = mark_origin->geometry();
    rect_mark_origin.moveTo(rect_mark_origin.x() + 33, rect_mark_origin.y() + 55);
    mark_origin->show();

    where_mark_origin = current_set_city;
    citylabel1_->setText(QString::fromStdString(idmap->getCityStr(current_set_city)));
}

void MyLabel::destination_action_triggered()
{
    double x = mouse_x_, y = mouse_y_;
    double city_x = city_pos_[current_set_city].first, city_y = city_pos_[current_set_city].second;
    double w_ratio = static_cast<double>(current_qsize_.width()) / static_cast<double>(origin_qsize_.width());
    double h_ratio = static_cast<double>(current_qsize_.height()) / static_cast<double>(origin_qsize_.height());
    city_x *= w_ratio;
    city_y *= h_ratio;

    mark_destination->move(city_x - 33, city_y - 55);
    rect_mark_destination = mark_destination->geometry();
    rect_mark_destination.moveTo(rect_mark_destination.x() + 33, rect_mark_destination.y() + 55);
    mark_destination->show();

    where_mark_destination = current_set_city;
    citylabel2_->setText(QString::fromStdString(idmap->getCityStr(current_set_city)));
}

void MyLabel::transfer_action_triggered()
{
    double x = mouse_x_, y = mouse_y_;
    double city_x = city_pos_[current_set_city].first, city_y = city_pos_[current_set_city].second;
    double w_ratio = static_cast<double>(current_qsize_.width()) / static_cast<double>(origin_qsize_.width());
    double h_ratio = static_cast<double>(current_qsize_.height()) / static_cast<double>(origin_qsize_.height());
    city_x *= w_ratio;
    city_y *= h_ratio;

    mark_transfer[current_set_city]->move(city_x - 33, city_y - 55);
    rect_mark_transfer[current_set_city] = mark_transfer[current_set_city]->geometry();
    rect_mark_transfer[current_set_city].moveTo(rect_mark_transfer[current_set_city].x() + 33, rect_mark_transfer[current_set_city].y() + 55);
    mark_transfer[current_set_city]->show();

    transfer_city.push_back(current_set_city);
    has_mark_transfer[current_set_city] = true;

    QString temp = "途经城市：";
    for(auto city : transfer_city)
    {
        temp += QString::fromStdString(idmap->getCityStr(city)) + " ";
    }
    transfercityBrowser_->setText(temp);
    /*for(auto city : transfer_city)
    {
        qDebug() << city;
    }*/
}

void MyLabel::delete_transfer_city(int i)
{
    for (auto iter = transfer_city.begin(); iter != transfer_city.end(); iter++)
    {
        if (i == *iter)
        {
            transfer_city.erase(iter);
            /*for(auto city : transfer_city)
            {
                qDebug() << city;
            }*/
            return;
        }
    }
}

std::vector<int> MyLabel::getplan()
{
    std::vector<int> temp;
    temp.push_back(where_mark_origin);
    for (const auto &i : transfer_city)
    // while(!transfer_city.empty())
    {
        temp.push_back(i);
        // temp.push_back(transfer_city.front());
        // transfer_city.pop_front();
    }
    temp.push_back(where_mark_destination);
    return temp;
}

void MyLabel::initializMyLabel(IDMap *a, QLabel *label1, QLabel *label2, QTextBrowser *browser)
{
    idmap = a;
    citylabel1_ = label1;
    citylabel2_ = label2;
    transfercityBrowser_ = browser;
    current_qsize_ = origin_qsize_ = this->size();

    rightbutton_menu = new QMenu(this);
    rightbutton_menu->setStyleSheet(
                "\
                QMenu {\
                \
                background-color:rgb(240,240,240);\
                border: 1px solid rgb(0,0,0);\
                }\
            QMenu::item {\
                color: rgb(0,0,0);\
                background-color:rgb(240,240,240);\
                 }\
            QMenu::item:selected { \
                background-color:rgb(200,200,200);\
                }\
               ");
    QAction *add_origin = new QAction(rightbutton_menu);
    QAction *add_destination = new QAction(rightbutton_menu);
    QAction *add_transfer = new QAction(rightbutton_menu);

    add_origin->setText("起点");
    add_destination->setText("终点");
    add_transfer->setText("途径点");

    rightbutton_menu->addAction(add_origin);
    rightbutton_menu->addAction(add_destination);
    rightbutton_menu->addAction(add_transfer);

    connect(add_origin, SIGNAL(triggered()), this, SLOT(origin_action_triggered()));
    connect(add_destination, SIGNAL(triggered()), this, SLOT(destination_action_triggered()));
    connect(add_transfer, SIGNAL(triggered()), this, SLOT(transfer_action_triggered()));

    mark_origin = new QLabel(this);
    mark_origin->setPixmap(QPixmap(":/image/6.png"));
    mark_origin->setScaledContents(true);
    mark_origin->resize(66, 66);
    mark_origin->hide();
    where_mark_origin = -1;

    mark_destination = new QLabel(this);
    mark_destination->setPixmap(QPixmap(":/image/2.png"));
    mark_destination->setScaledContents(true);
    mark_destination->resize(66, 66);
    mark_destination->hide();
    where_mark_destination = -1;

    for (int i = 0; i < 31; i++)
    {
        mark_transfer[i] = new QLabel(this);
        mark_transfer[i]->setPixmap(QPixmap(":/image/4.png"));
        mark_transfer[i]->setScaledContents(true);
        mark_transfer[i]->resize(66, 66);
        mark_transfer[i]->hide();
        has_mark_transfer[i] = false;
    }

    initialize_citymap_pos();
}

void MyLabel::UpdateColAndRowMap()
{
    double current_width = current_qsize_.width();
    double current_height = current_qsize_.height();
    double origin_width = origin_qsize_.width();
    double origin_height = origin_qsize_.height();

    for (int i = 0; i < sizeof(origin_col_map_) / sizeof(double); i++)
        col_map_[i] = origin_col_map_[i] * current_width / origin_width;

    for (int i = 0; i < sizeof(origin_row_map_) / sizeof(double); i++)
        row_map_[i] = origin_row_map_[i] * current_height / origin_height;
}

int MyLabel::judge_mouse_pos()
{
    UpdateColAndRowMap();
    if (mouse_x_ > col_map_[0])
    {
        if (mouse_x_ < col_map_[1])
        {
            if (mouse_y_ < row_map_[0])
            {
                if (in_city_range(6))
                    return 6;
            }
            else if (mouse_y_ > row_map_[2] && mouse_y_ < row_map_[3])
            {
                if (in_city_range(30))
                    return 30;
            }
        }
        else if (mouse_x_ > col_map_[2] && mouse_x_ < col_map_[3])
        {
            if (mouse_y_ > row_map_[1] && mouse_y_ < row_map_[2])
            {
                if (in_city_range(25))
                    return 25;
                if (in_city_range(7))
                    return 7;
            }
            else if (mouse_y_ > row_map_[2] && mouse_y_ < row_map_[3])
            {
                if (in_city_range(23))
                    return 23;
            }
            else if (mouse_y_ > row_map_[3] && mouse_y_ < row_map_[4])
            {
                if (in_city_range(24))
                    return 24;
            }
        }
        else if (mouse_x_ > col_map_[3] && mouse_x_ < col_map_[4])
        {
            if (mouse_y_ > row_map_[0] && mouse_y_ < row_map_[1])
            {
                if (in_city_range(8))
                    return 8;
            }
            else if (mouse_y_ > row_map_[1] && mouse_y_ < row_map_[2])
            {
                if (in_city_range(10))
                    return 10;
            }
            else if (mouse_y_ > row_map_[2] && mouse_y_ < row_map_[3])
            {
                if (in_city_range(28))
                    return 28;
            }
            else if (mouse_y_ > row_map_[3] && mouse_y_ < row_map_[4])
            {
                if (in_city_range(20))
                    return 20;
            }
            else if (mouse_y_ > row_map_[4])
            {
                if (in_city_range(19))
                    return 19;
            }
        }
        else if (mouse_x_ > col_map_[4] && mouse_x_ < col_map_[5])
        {
            if (mouse_y_ > row_map_[0] && mouse_y_ < row_map_[1])
            {
                if (in_city_range(5))
                    return 5;
                if (in_city_range(9))
                    return 9;
                if (in_city_range(1))
                    return 1;
                if (in_city_range(29))
                    return 29;
                if (in_city_range(26))
                    return 26;
            }
            else if (mouse_y_ > row_map_[1] && mouse_y_ < row_map_[2])
            {
                if (in_city_range(0))
                    return 0;
                if (in_city_range(11))
                    return 11;
            }
            else if (mouse_y_ > row_map_[2] && mouse_y_ < row_map_[3])
            {
                if (in_city_range(12))
                    return 12;
                if (in_city_range(22))
                    return 22;
                if (in_city_range(21))
                    return 21;
                if (in_city_range(17))
                    return 17;
            }
            else if (mouse_y_ > row_map_[4])
            {
                if (in_city_range(16))
                    return 16;
                if (in_city_range(18))
                    return 18;
            }
        }
        else if (mouse_x_ > col_map_[5] && mouse_x_ < col_map_[6])
        {
            if (mouse_y_ < row_map_[0])
            {
                if (in_city_range(3))
                    return 3;
                if (in_city_range(4))
                    return 4;
                if (in_city_range(2))
                    return 2;
            }
            else if (mouse_y_ > row_map_[2] && mouse_y_ < row_map_[3])
            {
                if (in_city_range(13))
                    return 13;
                if (in_city_range(27))
                    return 27;
                if (in_city_range(14))
                    return 14;
            }
            else if (mouse_y_ > row_map_[3] && mouse_y_ < row_map_[4])
            {
                if (in_city_range(15))
                    return 15;
            }
        }
    }
    // for (int i = 0; i < 31; i++)
    //     if (in_city_range(i))
    //         return i;
    return -1;
}

void MyLabel::initialize_citymap_pos()
{
    // 900 650图片下
    city_pos_[0] = std::make_pair(674, 323);  //济南
    city_pos_[1] = std::make_pair(631, 291);  //石家庄
    city_pos_[2] = std::make_pair(768, 172);  //长春
    city_pos_[3] = std::make_pair(778, 135);  //哈尔滨
    city_pos_[4] = std::make_pair(749, 212);  //沈阳
    city_pos_[5] = std::make_pair(580, 249);  //呼和浩特
    city_pos_[6] = std::make_pair(234, 179);  //乌鲁木齐
    city_pos_[7] = std::make_pair(465, 340);  //兰州
    city_pos_[8] = std::make_pair(500, 294);  //银川
    city_pos_[9] = std::make_pair(597, 301);  //太原
    city_pos_[10] = std::make_pair(545, 368); //西安
    city_pos_[11] = std::make_pair(625, 356); //郑州
    city_pos_[12] = std::make_pair(686, 400); //合肥
    city_pos_[13] = std::make_pair(718, 399); //南京
    city_pos_[14] = std::make_pair(736, 423); //杭州
    city_pos_[15] = std::make_pair(734, 499); //福州
    city_pos_[16] = std::make_pair(632, 563); //广州
    city_pos_[17] = std::make_pair(668, 459); //南昌
    city_pos_[18] = std::make_pair(578, 624); //海口
    city_pos_[19] = std::make_pair(541, 571); //南宁
    city_pos_[20] = std::make_pair(510, 506); //贵阳
    city_pos_[21] = std::make_pair(620, 471); //长沙
    city_pos_[22] = std::make_pair(641, 430); //武汉
    city_pos_[23] = std::make_pair(465, 433); //成都
    city_pos_[24] = std::make_pair(440, 533); //昆明
    city_pos_[25] = std::make_pair(430, 326); //西宁
    city_pos_[26] = std::make_pair(672, 273); //天津
    city_pos_[27] = std::make_pair(759, 407); //上海
    city_pos_[28] = std::make_pair(505, 451); //重庆
    city_pos_[29] = std::make_pair(657, 254); //北京
    city_pos_[30] = std::make_pair(246, 435); //拉萨
    // city_pos_[0][0] = 674;city_pos_[0][1] = 323;  //济南
    // city_pos_[1][0] = 631;city_pos_[1][1] = 291;  //石家庄
    // city_pos_[2][0] = 768;city_pos_[2][1] = 172;  //长春
    // city_pos_[3][0] = 778;city_pos_[3][1] = 135;  //哈尔滨
    // city_pos_[4][0] = 749;city_pos_[4][1] = 212;  //沈阳
    // city_pos_[5][0] = 580;city_pos_[5][1] = 249;  //呼和浩特
    // city_pos_[6][0] = 234;city_pos_[6][1] = 179;  //乌鲁木齐
    // city_pos_[7][0] = 465;city_pos_[7][1] = 340;  //兰州
    // city_pos_[8][0] = 500;city_pos_[8][1] = 294;  //银川
    // city_pos_[9][0] = 597;city_pos_[9][1] = 301;  //太原
    // city_pos_[10][0] = 545;city_pos_[10][1] = 368;//西安
    // city_pos_[11][0] = 625;city_pos_[11][1] = 356;//郑州
    // city_pos_[12][0] = 686;city_pos_[12][1] = 400;//合肥
    // city_pos_[13][0] = 718;city_pos_[13][1] = 399;//南京
    // city_pos_[14][0] = 736;city_pos_[14][1] = 423;//杭州
    // city_pos_[15][0] = 734;city_pos_[15][1] = 499;//福州
    // city_pos_[16][0] = 632;city_pos_[16][1] = 563;//广州
    // city_pos_[17][0] = 668;city_pos_[17][1] = 459;//南昌
    // city_pos_[18][0] = 578;city_pos_[18][1] = 624;//海口
    // city_pos_[19][0] = 541;city_pos_[19][1] = 571;//南宁
    // city_pos_[20][0] = 510;city_pos_[20][1] = 506;//贵阳
    // city_pos_[21][0] = 620;city_pos_[21][1] = 471;//长沙
    // city_pos_[22][0] = 641;city_pos_[22][1] = 430;//武汉
    // city_pos_[23][0] = 465;city_pos_[23][1] = 433;//成都
    // city_pos_[24][0] = 440;city_pos_[24][1] = 533;//昆明
    // city_pos_[25][0] = 430;city_pos_[25][1] = 326;//西宁
    // city_pos_[26][0] = 672;city_pos_[26][1] = 273;//天津
    // city_pos_[27][0] = 759;city_pos_[27][1] = 407;//上海
    // city_pos_[28][0] = 505;city_pos_[28][1] = 451;//重庆
    // city_pos_[29][0] = 657;city_pos_[29][1] = 254;//北京
    // city_pos_[30][0] = 246;city_pos_[30][1] = 435;//拉萨

    origin_col_map_[0] = 218;
    origin_col_map_[1] = 267;
    origin_col_map_[2] = 414;
    origin_col_map_[3] = 487;
    origin_col_map_[4] = 559;
    origin_col_map_[5] = 702;
    origin_col_map_[6] = 799;

    origin_row_map_[0] = 230;
    origin_row_map_[1] = 310;
    origin_row_map_[2] = 382;
    origin_row_map_[3] = 485;
    origin_row_map_[4] = 550;
}

bool MyLabel::in_city_range(int i)
{
    // qDebug() << std::pow(mouse_x_ - city_pos_[i].first, 2) + std::pow(mouse_y_ - city_pos_[i].second, 2) - r_city * r_city << endl;
    double x = mouse_x_, y = mouse_y_;
    double city_x = city_pos_[i].first, city_y = city_pos_[i].second;
    double w_ratio = static_cast<double>(current_qsize_.width()) / static_cast<double>(origin_qsize_.width());
    double h_ratio = static_cast<double>(current_qsize_.height()) / static_cast<double>(origin_qsize_.height());
    double r = r_city * std::min(w_ratio, h_ratio);
    city_x *= w_ratio;
    city_y *= h_ratio;

    return std::pow(x - city_x, 2.0) + std::pow(y - city_y, 2.0) <= r * r;
    // int x = ev->x();
    // int y = ev->y();
    //  qDebug() << this->size() << endl
    //           << this->origin_pixmap.size() << endl;
}

void MyLabel::MoveWhenResize(int type, int index, QLabel *target, const QSize &before_size, const QSize &after_size)
{
    struct Point
    {
        double x;
        double y;
    };

    static Point s_diff{}, e_diff{}, t_diff[31]{};
    Point *temp;

    if (type == 0) // start
        temp = &s_diff;
    else if (type == 1) // end
        temp = &e_diff;
    else // transfer
        temp = &t_diff[index];

    QRectF icon_rect = target->geometry();
    icon_rect.moveTo(icon_rect.x() + 33, icon_rect.y() + 55);

    double x_ratio = static_cast<double>(icon_rect.x()) / static_cast<double>(before_size.width());
    double y_ratio = static_cast<double>(icon_rect.y()) / static_cast<double>(before_size.height());

    double after_x = (after_size.width() * x_ratio); // 消除浮点转化为整数时的累积误差(尚有偏差)
    double after_y = (after_size.height() * y_ratio);
    int diff_x = after_x;
    int diff_y = after_y;

    temp->x += after_x - diff_x;
    temp->y += after_y - diff_y;

    int adjust_x = static_cast<int>(temp->x);
    int adjust_y = static_cast<int>(temp->y);

    if (adjust_x)
        temp->x -= adjust_x;

    if (adjust_y)
        temp->y -= adjust_y;

    // qDebug() << temp->x << " " << temp->y  << endl;
    icon_rect.moveTo((after_size.width() * x_ratio) + adjust_x, (after_size.height() * y_ratio) + adjust_y);
    icon_rect.moveTo(icon_rect.x() - 33, icon_rect.y() - 55);

    target->setGeometry(icon_rect.x(), icon_rect.y(), icon_rect.width(), icon_rect.height());
}

void MyLabel::resizeEvent(QResizeEvent *ev)
{
    current_qsize_ = ev->size();
    // qDebug() << current_qsize_ << endl;
    // qDebug() << mark_origin->geometry() << endl;

    MoveWhenResize(0, 0, mark_origin, ev->oldSize(), ev->size());
    MoveWhenResize(1, 0, mark_destination, ev->oldSize(), ev->size());

    for (int i = 0; i < 31; i++)
    {
        if (has_mark_transfer[i])
            MoveWhenResize(2, i, mark_transfer[i], ev->oldSize(), ev->size());
    }

    this->setPixmap(origin_pixmap.scaled(ev->size().width(),
                                         ev->size().height(),
                                         Qt::KeepAspectRatio,
                                         Qt::FastTransformation));
}
#endif // SRC_MYLABEL
