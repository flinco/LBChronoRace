#include "chronoracelogo.h"

void ChronoRaceLogo::loadLogo()
{
    emit logoLoaded(this->uiElement);
}

void ChronoRaceLogo::removeLogo()
{
    emit logoRemoved(this->uiElement);
}
