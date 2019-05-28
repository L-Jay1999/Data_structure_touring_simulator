#include "simulator.h"

#pragma execution_character_set("utf-8")

Simulator::Simulator(QObject *parent) : QObject(parent)
{
}

void Simulator::initialize(QLCDNumber *lcd1, QLCDNumber *lcd2, MyMap *map, CityGraph *cg, Traveller *t)
{
    day_LCD_ = lcd1;
    hour_LCD_ = lcd2;
    map_ = map;
    map_->initialize(cg, t);
    set_flush_rate(1, 20); //这里可以改速率
    timer_.setInterval(display_interval_ms_);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(onTimerOut()));
}

void Simulator::ready_for_simulate()
{
    display_day_ = map_->traveller_->get_init_time().GetDay() + 1;
    display_hour_ = map_->traveller_->get_init_time().GetHour();
    day_LCD_->display(display_day_);
    hour_LCD_->display(display_hour_);
    //qDebug() << simulate_onehour_s_ << display_interval_ms_ << flush_per_hour_;
    map_->ready_for_simulate();
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
            current_pathnode_++;
            flush_count_ = 0;
            if (current_pathnode_ == map_->traveller_path_.GetLen())
            {
                timer_.stop();
                is_finished = true;
                return;
            }

            hour_left_ = map_->traveller_->get_stay_hours(*(map_->citygraph_), current_pathnode_);
            if (hour_left_ == 0)
            {
                current_states_ = OFF;
                hour_left_ = map_->traveller_->get_off_hours(*(map_->citygraph_), current_pathnode_);
            }
            else
                current_states_ = STAY;
        }
    }
}

void Simulator::start()
{
    if (!is_start_ && !is_finished)
    {
        hour_left_ = map_->traveller_->get_stay_hours(*(map_->citygraph_), 0);
        if (hour_left_ == 0)
        {
            current_states_ = OFF;
            hour_left_ = map_->traveller_->get_off_hours(*(map_->citygraph_), 0);
        }
        else
            current_states_ = STAY;
        is_start_ = true;
        is_finished = false;
        timer_.start();
    }
}

void Simulator::stop()
{
    if (is_start_ && !is_finished)
        timer_.stop();
}

void Simulator::continuing()
{
    if (is_start_ && !is_finished)
        timer_.start();
}

void Simulator::reset()
{
    if (is_start_ || is_finished)
    {
        timer_.stop();
        display_day_ = map_->traveller_->get_init_time().GetDay() + 1;
        display_hour_ = map_->traveller_->get_init_time().GetHour();
        day_LCD_->display(display_day_);
        hour_LCD_->display(display_hour_);
        is_start_ = false;
        is_finished = false;
        flush_count_ = 0;
        current_pathnode_ = 0;
        map_->reset();
    }
}
