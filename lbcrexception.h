/*****************************************************************************
 * Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#ifndef LBEXCEPTION_H
#define LBEXCEPTION_H

#include <QException>
#include <QString>

class ChronoRaceException : public QException
{
private:
    QString message { "No message" };
public:
    explicit ChronoRaceException(QString const &message) : message(message) { };
    explicit ChronoRaceException(char const *message) : message(message) { };

    void raise() const override;
    ChronoRaceException *clone() const override;

    void setMessage(QString const &newMessage);
    QString &getMessage();
};

#endif // LBEXCEPTION_H
