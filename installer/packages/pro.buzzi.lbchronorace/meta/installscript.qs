/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

function Component()
{
    // File extension registration
    component.loaded.connect(this, addRegisterFileCheckBox);
    
    component.userInterface("RegisterFileCheckBoxForm").windowTitle.text = qsTranslate("installscript", "Form");
    component.userInterface("RegisterFileCheckBoxForm").RegisterFileCheckBox.text = qsTranslate("installscript", "Register ChronoRace Data file extension: ");
}

// called as soon as the component was loaded
addRegisterFileCheckBox = function()
{
    // don't show when updating or uninstalling
    if (installer.isInstaller()) {
        if (installer.addWizardPageItem(component, "RegisterFileCheckBoxForm", QInstaller.TargetDirectory)) {
            component.userInterface("RegisterFileCheckBoxForm").RegisterFileCheckBox.text =
                component.userInterface("RegisterFileCheckBoxForm").RegisterFileCheckBox.text + "crd";
        }
    }
}

// here we are creating the operation chain which will be processed at the real installation part later
Component.prototype.createOperations = function()
{
    try {
        // call the base create operations function
        component.createOperations();

        // Target directory selection
        if (systemInfo.productType === "windows") {
            component.addOperation("CreateShortcut",
                "@TargetDir@\\LBChronoRace.exe", "@StartMenuDir@\\LBChronoRace.lnk",
                "workingDirectory=@TargetDir@", "iconPath=@TargetDir@\\LBChronoRace.exe",
                "iconId=0", "description=" + qsTranslate("installscript", "Start LBChronoRace"));
            component.addOperation("CreateShortcut",
                "@TargetDir@\\LBChronoRace.exe", "@DesktopDir@\\LBChronoRace.lnk",
                "workingDirectory=@TargetDir@", "iconPath=@TargetDir@\\LBChronoRace.exe",
                "iconId=0", "description=" + qsTranslate("installscript", "Start LBChronoRace"));
        }

        // File extension registration
        if (component.userInterface("RegisterFileCheckBoxForm")) {
            var isRegisterFileChecked = component.userInterface("RegisterFileCheckBoxForm").RegisterFileCheckBox.checked;
        }
        if (installer.value("os") === "win") {
            var iconId = 0;
            component.addOperation("RegisterFileType",
                                   "crd",
                                   "@TargetDir@\\LBChronoRace.exe \"%1\"",
                                   "ChronoRace Data file type",
                                   "application/x-binary",
                                   "@TargetDir@\\LBChronoRaceCRD.ico",
                                   "ProgId=pro.buzzi.lbchronorace.crd");
        }
    } catch (e) {
        console.log(e);
    }
}
