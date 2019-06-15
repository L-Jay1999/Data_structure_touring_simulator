#include "simulator.h"

#pragma execution_character_set("utf-8")

Simulator::Simulator(QObject *parent) : QObject(parent)
{
}

void Simulator::initialize(QLCDNumber *lcd1, QLCDNumber *lcd2, MyMap *map, QLabel *label, IDMap *idmap, CityGraph *cg, Traveller *t)
{
    day_LCD_ = lcd1;
    hour_LCD_ = lcd2;
    display_label_ = label;
    idmap_ = idmap;
    map_ = map;
    map_->initialize(cg, t);
    set_flush_rate(1, 10); //这里可以改速率
    timer_.setInterval(display_interval_ms_);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimerOut()));
}

void Simulator::ready_for_simulate()
{
    display_day_ = map_->traveller_->get_init_time().get_day() + 1;
    display_hour_ = map_->traveller_->get_init_time().get_hour();
    day_LCD_->display(display_day_);
    hour_LCD_->display(display_hour_);
    //qDebug() << simulate_onehour_s_ << display_interval_ms_ << flush_per_hour_;
    map_->ready_for_simulate();
    display_label_->setText(QString::fromStdString(idmap_->getCityStr(map_->traveller_path_.getNode(0).former_city)));
    reset();
}

void Simulator::set_flush_rate(int simulate_onehour_s_, int display_interval_ms_)
{
    this->simulate_onehour_s_ = simulate_onehour_s_;
    this->display_interval_ms_ = display_interval_ms_;
    flush_per_hour_ = simulate_onehour_s_ * 1000 / display_interval_ms_;
    map_->set_flush_per_hour(flush_per_hour_);
}

void Simulator::onTimerOut()
{
    flush_count_++;
    if (current_states_ == OFF)
        map_->move(current_pathnode_, hour_left_, flush_count_);
    if (flush_count_ % flush_per_hour_ == 0)
    {
        hour_left_--;

        hour_LCD_->display((++display_hour_) % 24);
        if (display_hour_ % 24 == 0)
            day_LCD_->display(++display_day_);

        if (hour_left_ == 0)
        {
            flush_count_ = 0;

            if (current_states_ == STAY)
            {
                hour_left_ = map_->traveller_->get_off_hours(*(map_->citygraph_), current_pathnode_);
                current_states_ = OFF;
            }
            else
            {
                current_pathnode_++;

                if (current_pathnode_ == map_->traveller_path_.get_len())
                {
                    display_label_->setText(QString::fromStdString(idmap_->getCityStr(map_->traveller_path_.getNode(current_pathnode_-1).current_city)));
                    timer_.stop();
                    is_finished_ = true;
                    return;
                }
                else
                    map_->reset_image(current_pathnode_);

                hour_left_ = map_->traveller_->get_stay_hours(*(map_->citygraph_), current_pathnode_);
                if (hour_left_ == 0)
                {
                    display_label_->setText(QString::fromStdString(idmap_->getCityStr(map_->traveller_path_.getNode(current_pathnode_).former_city) + " -> " + idmap_->getCityStr(map_->traveller_path_.getNode(current_pathnode_).current_city)));
                    hour_left_ = map_->traveller_->get_off_hours(*(map_->citygraph_), current_pathnode_);
                    current_states_ = OFF;
                }
                else
                {
                    display_label_->setText(QString::fromStdString(idmap_->getCityStr(map_->traveller_path_.getNode(current_pathnode_).former_city)));
                    current_states_ = STAY;
                }
            }
        }
    }
}

void Simulator::start()
{
    if (!is_start_ && !is_finished_)
    {
        hour_left_ = map_->traveller_->get_stay_hours(*(map_->citygraph_), 0);
        if (hour_left_ == 0)
        {
            display_label_->setText(QString::fromStdString(idmap_->getCityStr(map_->traveller_path_.getNode(current_pathnode_).former_city) + " -> " + idmap_->getCityStr(map_->traveller_path_.getNode(current_pathnode_).current_city)));
            current_states_ = OFF;
            hour_left_ = map_->traveller_->get_off_hours(*(map_->citygraph_), 0);
        }
        else
            current_states_ = STAY;
        is_start_ = true;
        is_finished_ = false;
        timer_.start();
    }
}

void Simulator::stop()
{
    if (is_start_ && !is_finished_)
        timer_.stop();
}

void Simulator::continuing()
{
    if (is_start_ && !is_finished_)
        timer_.start();
}

void Simulator::reset()
{
    if (is_start_ || is_finished_)
    {
        timer_.stop();
        display_day_ = map_->traveller_->get_init_time().get_day() + 1;
        display_hour_ = map_->traveller_->get_init_time().get_hour();
        day_LCD_->display(display_day_);
        hour_LCD_->display(display_hour_);
        display_label_->setText(QString::fromStdString(idmap_->getCityStr(map_->traveller_path_.getNode(0).former_city)));
        is_start_ = false;
        is_finished_ = false;
        flush_count_ = 0;
        current_pathnode_ = 0;
        map_->reset();
    }
}
