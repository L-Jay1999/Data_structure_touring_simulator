#ifndef SAVE_AT_EXIT_H
#define SAVE_AT_EXIT_H

class Traveller;

void setTravellerPtr(Traveller *traveller) noexcept;

void SaveDataOnExit();

void SaveDataOnExit(int sig);

void setSignalHandlers();

#endif // SAVE_AT_EXIT_H
