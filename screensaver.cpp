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

#include "screensaver.hpp"

void ScreenSaver::inhibit(bool inhibit)
{
    if (inhibit)
        reqCount++;
    else if (reqCount > 0)
        reqCount--;

#if defined(Q_OS_WIN)
    if (reqCount) {
        if (execState_ == static_cast<EXECUTION_STATE>(NULL)) {
            execState_ = SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
        }
    } else {
        if (execState_ != static_cast<EXECUTION_STATE>(NULL)) {
            SetThreadExecutionState(ES_CONTINUOUS | execState_);
            execState_ = static_cast<EXECUTION_STATE>(NULL);
        }
    }
#elif defined(Q_OS_MACOS)
    if (reqCount) {
        if (s_power_assertion_ == kIOPMNullAssertionID) {
            if (IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleSystemSleep,
                                            kIOPMAssertionLevelOn, CFSTR("Metashape processing"),
                                            &s_power_assertion_) != kIOReturnSuccess) {
                s_power_assertion_ = kIOPMNullAssertionID;
            }
        }
    } else {
        if (s_power_assertion_ != kIOPMNullAssertionID) {
            IOPMAssertionRelease(s_power_assertion_);
            s_power_assertion_ = kIOPMNullAssertionID;
        }
    }
#elif defined(Q_OS_LINUX)
    if (QDBusConnection bus_ = QDBusConnection::sessionBus();
        !bus_.isConnected()) {
        qDebug() << "No connection to DBus";
    } else if (QDBusInterface sessionManagerInterface(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/org/freedesktop/ScreenSaver"), QStringLiteral("org.freedesktop.ScreenSaver"), bus_, this);
               !sessionManagerInterface.isValid()) {
        qDebug() << "Invalid DBus interface";
    } else if (reqCount) {
        if (cookie_ != 0) {
            qDebug() << "ScreenSaver" << (inhibit ? "already" : "still") << "inhibited";
        } else if (QDBusReply<uint> reply = sessionManagerInterface.call("Inhibit", QStringLiteral(LBCHRONORACE_NAME), "Timekeeping");
                   reply.isValid()) {
            cookie_ = reply.value();
            qDebug() << "ScreenSaver disabled";
        } else {
            QDBusError error = reply.error();
            qDebug() << error.message() << error.name();
        }
    } else {
        if (cookie_ == 0) {
            qDebug() << "ScreenSaver not inhibited";
        } else if (QDBusReply<void> reply = sessionManagerInterface.call("UnInhibit", cookie_);
                   reply.isValid()) {
            cookie_ = 0u;
            qDebug() << "ScreenSaver enabled";
        } else {
            QDBusError error = reply.error();
            qDebug() << error.message() << error.name();
        }
    }
#endif
}
