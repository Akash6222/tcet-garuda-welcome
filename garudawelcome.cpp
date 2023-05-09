/**********************************************************************
 *  tcetwelcome.cpp
 **********************************************************************
 * Copyright (C) 2015 tcet Authors
 *
 * Authors: Adrian
 *          Paul David Callahan
 *          Dolphin Oracle
 *          tcet Linux <http://tcetlinux.org>
 *
 * This file is part of tcet-welcome.
 *
 * tcet-welcome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tcet-welcome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tcet-welcome.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "tcetwelcome.h"
#include "ui_tcetwelcome.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QUrl>

tcetwelcome::tcetwelcome(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::tcetwelcome)
{
    ui->setupUi(this);
    setup();
}

tcetwelcome::~tcetwelcome()
{
    delete ui;
}

bool tcetwelcome::isFirstLaunch()
{
    QSettings settings;

    if (settings.value("tcet-welcome/firstRun").isNull()) {
        settings.setValue("tcet-welcome/firstRun", false);

        QProcess proc;
        proc.start("last", { "reboot" });
        proc.waitForFinished();
        QString out = proc.readAllStandardOutput();
        if (out.count("\n") <= 3)
            return true;
    }
    return false;
}

// do various setup tasks
void tcetwelcome::setup()
{
    version = getVersion();
    codename = getCodename();
    description = getDescription();
    QFont f("Noto Sans", 16, QFont::Bold);
    ui->labelTitle->setFont(f);
    ui->labelTitle->setText(description + " " + codename);
    this->setWindowTitle(tr("tcet Welcome"));
    if (system("[ -f ~/.config/autostart/tcet-welcome.desktop ]") == 0) {
        ui->checkBox->setChecked(true);
    } else {
        ui->checkBox->setChecked(false);
    }
    // if running live
    QString test = runCmd("df -T / |tail -n1 |awk '{print $2}'").output;
    qDebug() << test;
    if (test == "aufs" || test == "overlay") {
        ui->checkBox->hide();
        ui->pushButton_setupassistant->hide();
    } else {
        ui->buttonInstalltcet->hide();
        ui->buttonChroot->hide();
        ui->buttonBootRepair->hide();
        ui->buttonTimeshift->setText("BTRFS Assistant");
        // ui->buttonTimeshift->hide();

        if (isFirstLaunch()) {
            if (QFile::exists("/usr/bin/setup-assistant")) {
                system("VERSION=2 setup-assistant");
            }
        }
    }

    /*
        // We want to sync the widths of every button in tab_3
        // Otherwise the alignment between the 2 groupboxes is non existent and it makes dr460nf1r3 and I sad
        int width = 0;
        auto buttons = this->ui->scrollArea->findChildren<QToolButton*>();
        for (auto element : qAsConst(buttons)) {
            // Let QT calculate the correct width for us.
            int b_width = element->sizeHint().width();
            if (b_width > width)
                width = b_width;
        }

        for (auto button : qAsConst(buttons)) {
            // Now that we know the size, we can force it.
            button->setMinimumWidth(width);
            // We need to force the maximum height here, otherwise weird things happen if you resize the window.
            // This would not be necessary if we used "fixed" size qtoolbuttons, but in that case the qtoolbuttons would be a different height sometimes
            button->setMaximumHeight(button->sizeHint().height());
        }

        //this->adjustSize();
    */
}

// Util function for getting bash command output and error code
Result tcetwelcome::runCmd(QString cmd, bool include_stderr)
{
    QProcess proc(this);
    if (include_stderr)
        proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start("/bin/bash", QStringList() << "-c" << cmd);
    // Wait forever
    proc.waitForFinished(-1);
    Result result = { proc.exitCode(), proc.readAll().trimmed() };
    return result;
}

// Get version of the program
QString tcetwelcome::getVersion()
{
    QString cmd = QString("lsb_release -r | cut -f2");
    return runCmd(cmd).output;
}

// Get distribution name and codename of the program
QString tcetwelcome::getCodename()
{
    QString cmd = QString("lsb_release -c | cut -f2");
    return runCmd(cmd).output;
}

QString tcetwelcome::getDescription()
{
    QString cmd = QString("lsb_release -d | cut -f2");
    return runCmd(cmd).output;
}

// About button clicked
void tcetwelcome::on_buttonAbout_clicked()
{
    this->hide();
    QMessageBox msgBox(QMessageBox::NoIcon,
        tr("About tcet Welcome"), "<p align=\"center\"><b><h2>" + tr("tcet Welcome") + "</h2></b></p><p align=\"center\">" + tr("Version: ") + version + "</p><p align=\"center\"><h3>" + tr("Program for displaying a welcome screen in tcet Linux") + "</h3></p><p align=\"center\"><a href=\"http://www.tcetlinux.org/\">http://www.tcetlinux.org/</a><br /></p><p align=\"center\">" + tr("Copyright (c) tcet Linux") + "<br /><br /></p>", 0, this);
    msgBox.addButton(tr("License"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    if (msgBox.exec() == QMessageBox::AcceptRole) {
        QDesktopServices::openUrl(QUrl("https://www.gnu.org/licenses/gpl-3.0.html"));
    }
    this->show();
}

// Add/remove autostart at login
void tcetwelcome::on_checkBox_clicked(bool checked)
{
    if (checked) {
        system("cp /usr/share/applications/tcet-welcome.desktop ~/.config/autostart/tcet-welcome.desktop");
    } else {
        system("rm ~/.config/autostart/tcet-welcome.desktop >/dev/null 2>&1");
    }
}

// Launch Forum in browser
void tcetwelcome::on_buttonForum_clicked()
{
    QDesktopServices::openUrl(QUrl("https://forum.tcetlinux.org/"));
}

void tcetwelcome::on_buttonWebsite_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.tcetlinux.org/"));
}

void tcetwelcome::on_buttonGitlab_clicked()
{
    QDesktopServices::openUrl(QUrl("https://gitlab.com/groups/tcet-linux/"));
}

void tcetwelcome::on_buttonRepository_clicked()
{
    QDesktopServices::openUrl(QUrl("https://aur.chaotic.cx"));
}

void tcetwelcome::on_buttonTelegram_clicked()
{
    QDesktopServices::openUrl(QUrl("https://telegram.me/tcetlinux"));
}

void tcetwelcome::on_buttonTwitter_clicked()
{
    QDesktopServices::openUrl(QUrl("https://twitter.com/tcetlinux"));
}

void tcetwelcome::on_buttonBitwarden_clicked()
{
    QDesktopServices::openUrl(QUrl("https://bitwarden.tcetlinux.org/"));
}

void tcetwelcome::on_buttonPrivatebin_clicked()
{
    QDesktopServices::openUrl(QUrl("https://bin.tcetlinux.org/"));
}

void tcetwelcome::on_buttonCryptpad_clicked()
{
    QDesktopServices::openUrl(QUrl("https://pad.tcetlinux.org/"));
}

void tcetwelcome::on_buttonBigbluebutton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://meet.tcetlinux.org/"));
}

void tcetwelcome::on_buttonNextcloud_clicked()
{
    QDesktopServices::openUrl(QUrl("https://cloud.tcetlinux.org/"));
}

void tcetwelcome::on_buttonStatping_clicked()
{
    QDesktopServices::openUrl(QUrl("https://status.tcetlinux.org/"));
}

void tcetwelcome::on_buttonWhoogle_clicked()
{
    QDesktopServices::openUrl(QUrl("https://search.tcetlinux.org/"));
}

void tcetwelcome::on_buttonSearx_clicked()
{
    QDesktopServices::openUrl(QUrl("https://searx.tcetlinux.org/"));
}

void tcetwelcome::on_buttonWiki_clicked()
{
    QDesktopServices::openUrl(QUrl("https://wiki.tcetlinux.org/"));
}

void tcetwelcome::on_buttonElement_clicked()
{
    QDesktopServices::openUrl(QUrl("https://element.tcetlinux.org/"));
}

void tcetwelcome::on_buttonfaq_clicked()
{
    QDesktopServices::openUrl(QUrl("https://forum.tcetlinux.org/t/tcet-linux-faq/318/"));
}

void tcetwelcome::on_buttonDonate_clicked()
{
    QDesktopServices::openUrl(QUrl("https://tcetlinux.org/donate.html"));
}

void tcetwelcome::on_buttonIrc_clicked()
{
    QDesktopServices::openUrl(QUrl("https://irc.tcetlinux.org/"));
}

void tcetwelcome::on_buttonDiscord_clicked()
{
    QDesktopServices::openUrl(QUrl("https://tcetlinux.org/discord"));
}

void tcetwelcome::on_buttontcetSettings_clicked()
{
    if (!checkAndInstall("tcet-settings-manager"))
        return;
    this->hide();
    system("tcet-settings-manager");
    this->show();
}

void tcetwelcome::on_buttontcetAssistant_clicked()
{
    if (!checkAndInstall("tcet-assistant"))
        return;
    this->hide();
    system("tcet-assistant");
    this->show();
}

void tcetwelcome::on_buttontcetGamer_clicked()
{
    if (!checkAndInstall("tcet-gamer"))
        return;
    this->hide();
    system("tcet-gamer");
    this->show();
}

void tcetwelcome::on_buttonBootOptions_clicked()
{
    if (!checkAndInstall("tcet-boot-options"))
        return;
    this->hide();
    system("/usr/lib/tcet/pkexec-gui tcet-boot-options");
    this->show();
}

void tcetwelcome::on_buttonBootRepair_clicked()
{
    if (!checkAndInstall("tcet-boot-repair"))
        return;
    this->hide();
    system("/usr/lib/tcet/pkexec-gui tcet-boot-repair");
    this->show();
}

void tcetwelcome::on_buttonNetworkAssistant_clicked()
{
    if (!checkAndInstall("tcet-network-assistant"))
        return;
    this->hide();
    system("/usr/lib/tcet/pkexec-gui tcet-network-assistant");
    this->show();
}

void tcetwelcome::on_buttonTimeshift_clicked()
{
    if (!checkAndInstall("btrfs-assistant"))
        return;
    this->hide();
    system("/usr/lib/tcet/pkexec-gui btrfs-assistant");
    this->show();
}

void tcetwelcome::on_buttonPartition_clicked()
{
    this->hide();
    if (QFile::exists("/usr/bin/gnome-disk-utility")) {
        system("gnome-disk-utility");
    } else if (QFile::exists("/usr/bin/partitionmanager")) {
        system("partitionmanager");
    } else if (QFile::exists("/usr/bin/gparted")) {
        system("gparted");
    } else {
        if (checkAndInstall("gparted"))
            system("gparted");
    }

    this->show();
}

void tcetwelcome::on_buttonSystemCleaner_clicked()
{
    this->hide();
    if (QFile::exists("/usr/bin/stacer")) {
        system("stacer");
    } else if (QFile::exists("/usr/bin/bleachbit")) {
        system("bleachbit");
    } else if (QFile::exists("/usr/bin/sweeper")) {
        system("sweeper");
    } else {
        if (checkAndInstall("stacer"))
            system("stacer");
    }

    this->show();
}

void tcetwelcome::on_buttonSoftware_clicked()
{
    this->hide();
    if (QFile::exists("/usr/bin/pamac-manager")) {
        system("pamac-manager");
    } else if (QFile::exists("/usr/bin/bauh")) {
        system("bauh");
    } else if (QFile::exists("/usr/bin/octopi")) {
        system("/usr/bin/octopi");
    }

    this->show();
}

void tcetwelcome::on_buttonInstalltcet_clicked()
{
    this->hide();
    if (QFile::exists("/usr/bin/kvantummanager")) {
        system("/usr/bin/calamares_polkit %f -style kvantum");
    } else {
        system("sudo -E calamares");
    }
    this->show();
}

void tcetwelcome::on_buttonChroot_clicked()
{
    this->hide();
    system("/usr/lib/tcet/launch-terminal \"pkexec tcet-chroot -a; read -p 'Press enter to exit'\"");
    this->show();
}

void tcetwelcome::on_pushButton_setupassistant_clicked()
{
    if (!checkAndInstall("tcet-setup-assistant"))
        return;
    this->hide();
    system("setup-assistant");
    this->show();
}

bool tcetwelcome::checkAndInstall(QString package)
{
    // Not async, don't really care either tho, not my problem
    // Assumption: package is a safe string
    auto output = runCmd("pacman -Qq " + package, false);

    // If it's already installed, we are good to go
    if (output.output == package || output.output == package + "-git")
        return true;

    this->hide();
    runCmd("/usr/lib/tcet/launch-terminal \"/usr/lib/tcet/install-software " + package + "; read -p 'Press enter to exit'\"");

    // Checking pamac-installer's exit code alone is not enough. For some reason it decides to return 0 even tho it failed sometimes
    output = runCmd("pacman -Qq " + package, false);
    if (output.output == package)
        return true;
    else {
        this->show();
        return false;
    }
}
