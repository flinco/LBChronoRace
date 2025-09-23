/*****************************************************************************
 * Copyright (C) 2024 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#ifndef SCREENSAVER_HPP
#define SCREENSAVER_HPP

#include <QObject>

#if defined(Q_OS_WIN)
#include <Windows.h>
#include <WinBase.h>
#elif defined(Q_OS_MACOS)
#include <IOKit/pwr_mgt/IOPMLib.h>
#elif defined(Q_OS_LINUX)
#include <qdbusconnection.h>
#include <qdbusinterface.h>
#include <qdbusreply.h>
#else
#warning "Screensaver inhibition not implemented on this OS"
#endif


class ScreenSaver : public QObject
{
    Q_OBJECT

private:
    uint reqCount { 0u };
#if defined(Q_OS_WIN)
    EXECUTION_STATE execState_ { static_cast<EXECUTION_STATE>(NULL) };
#elif defined(Q_OS_MACOS)
    IOPMAssertionID s_power_assertion_ { kIOPMNullAssertionID };
#elif defined(Q_OS_LINUX)
    uint cookie_ { 0u };
#endif

public:
    void inhibit(bool inhibit);
};

#endif // SCREENSAVER_HPP
