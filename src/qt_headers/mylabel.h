#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QResizeEvent>
#include <QTextBrowser>
#include <utility>

#include <../headers/id_map.h>
#include <deque>
#include <vector>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = 0);
    void initializMyLabel(IDMap *a, QLabel *label1, QLabel *label2, QTextBrowser *browser);
    std::vector <int> get_plan();
    void setOriginPixmap();
    bool hasOriginPixmap();
    bool has_origin(){if(where_mark_origin != -1)return true;else return false;}
    bool has_destination(){if(where_mark_destination != -1)return true;else return false;}
protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void resizeEvent(QResizeEvent *ev);
signals:

public slots:
    void origin_action_triggered();
    void destination_action_triggered();
    void transfer_action_triggered();
private:
    int mouse_x_;
    int mouse_y_;

    void MoveWhenResize(int type, int index, QLabel *target, const QSize &before_size, const QSize &current_size);
    QMenu *rightbutton_menu;

    QLabel *mark_origin;
    QRect rect_mark_origin;
    int where_mark_origin;

    QLabel *mark_destination;
    QRect  rect_mark_destination;
    int where_mark_destination;

    QLabel *mark_transfer[31];
    QRect rect_mark_transfer[31];
    bool has_mark_transfer[31];

    int current_set_city;
    std::deque <int> transfer_city;

    std::pair<int, int> city_pos_[31];      //在图片大小为900x650情况下
    double origin_col_map_[7];
    double col_map_[7];
    double origin_row_map_[5];
    double row_map_[5];

    void UpdateColAndRowMap();
    QSize origin_qsize_;
    QSize current_qsize_;

    IDMap *idmap;

    void initialize_citymap_pos();
    bool in_city_range(int i);
    int judge_mouse_pos();
    void delete_transfer_city(int i);

    QPixmap origin_pixmap;

    QLabel *citylabel1_;
    QLabel *citylabel2_;
    QTextBrowser *transfercityBrowser_;
};

#endif // MYLABEL_H
