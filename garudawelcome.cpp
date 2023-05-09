/**********************************************************************
 *  garudawelcome.cpp
 **********************************************************************
 * Copyright (C) 2015 Garuda Authors
 *
 * Authors: Adrian
 *          Paul David Callahan
 *          Dolphin Oracle
 *          Garuda Linux <http://garudalinux.org>
 *
 * This file is part of garuda-welcome.
 *
 * garuda-welcome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * garuda-welcome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with garuda-welcome.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "garudawelcome.h"
#include "ui_garudawelcome.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QUrl>

garudawelcome::garudawelcome(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::garudawelcome)
{
    ui->setupUi(this);
    setup();
}

garudawelcome::~garudawelcome()
{
    delete ui;
}

bool garudawelcome::isFirstLaunch()
{
    QSettings settings;

    if (settings.value("garuda-welcome/firstRun").isNull()) {
        settings.setValue("garuda-welcome/firstRun", false);

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
void garudawelcome::setup()
{
    version = getVersion();
    codename = getCodename();
    description = getDescription();
    QFont f("Noto Sans", 16, QFont::Bold);
    ui->labelTitle->setFont(f);
    ui->labelTitle->setText(description + " " + codename);
    this->setWindowTitle(tr("Garuda Welcome"));
    if (system("[ -f ~/.config/autostart/garuda-welcome.desktop ]") == 0) {
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
        ui->buttonInstallGaruda->hide();
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
Result garudawelcome::runCmd(QString cmd, bool include_stderr)
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
QString garudawelcome::getVersion()
{
    QString cmd = QString("lsb_release -r | cut -f2");
    return runCmd(cmd).output;
}

// Get distribution name and codename of the program
QString garudawelcome::getCodename()
{
    QString cmd = QString("lsb_release -c | cut -f2");
    return runCmd(cmd).output;
}

QString garudawelcome::getDescription()
{
    QString cmd = QString("lsb_release -d | cut -f2");
    return runCmd(cmd).output;
}

// About button clicked
void garudawelcome::on_buttonAbout_clicked()
{
    this->hide();
    QMessageBox msgBox(QMessageBox::NoIcon,
        tr("About Garuda Welcome"), "<p align=\"center\"><b><h2>" + tr("Garuda Welcome") + "</h2></b></p><p align=\"center\">" + tr("Version: ") + version + "</p><p align=\"center\"><h3>" + tr("Program for displaying a welcome screen in Garuda Linux") + "</h3></p><p align=\"center\"><a href=\"http://www.garudalinux.org/\">http://www.garudalinux.org/</a><br /></p><p align=\"center\">" + tr("Copyright (c) Garuda Linux") + "<br /><br /></p>", 0, this);
    msgBox.addButton(tr("License"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    if (msgBox.exec() == QMessageBox::AcceptRole) {
        QDesktopServices::openUrl(QUrl("https://www.gnu.org/licenses/gpl-3.0.html"));
    }
    this->show();
}

// Add/remove autostart at login
void garudawelcome::on_checkBox_clicked(bool checked)
{
    if (checked) {
        system("cp /usr/share/applications/garuda-welcome.desktop ~/.config/autostart/garuda-welcome.desktop");
    } else {
        system("rm ~/.config/autostart/garuda-welcome.desktop >/dev/null 2>&1");
    }
}

// Launch Forum in browser
void garudawelcome::on_buttonForum_clicked()
{
    QDesktopServices::openUrl(QUrl("https://forum.garudalinux.org/"));
}

void garudawelcome::on_buttonWebsite_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.garudalinux.org/"));
}

void garudawelcome::on_buttonGitlab_clicked()
{
    QDesktopServices::openUrl(QUrl("https://gitlab.com/groups/garuda-linux/"));
}

void garudawelcome::on_buttonRepository_clicked()
{
    QDesktopServices::openUrl(QUrl("https://aur.chaotic.cx"));
}

void garudawelcome::on_buttonTelegram_clicked()
{
    QDesktopServices::openUrl(QUrl("https://telegram.me/garudalinux"));
}

void garudawelcome::on_buttonTwitter_clicked()
{
    QDesktopServices::openUrl(QUrl("https://twitter.com/garudalinux"));
}

void garudawelcome::on_buttonBitwarden_clicked()
{
    QDesktopServices::openUrl(QUrl("https://bitwarden.garudalinux.org/"));
}

void garudawelcome::on_buttonPrivatebin_clicked()
{
    QDesktopServices::openUrl(QUrl("https://bin.garudalinux.org/"));
}

void garudawelcome::on_buttonCryptpad_clicked()
{
    QDesktopServices::openUrl(QUrl("https://pad.garudalinux.org/"));
}

void garudawelcome::on_buttonBigbluebutton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://meet.garudalinux.org/"));
}

void garudawelcome::on_buttonNextcloud_clicked()
{
    QDesktopServices::openUrl(QUrl("https://cloud.garudalinux.org/"));
}

void garudawelcome::on_buttonStatping_clicked()
{
    QDesktopServices::openUrl(QUrl("https://status.garudalinux.org/"));
}

void garudawelcome::on_buttonWhoogle_clicked()
{
    QDesktopServices::openUrl(QUrl("https://search.garudalinux.org/"));
}

void garudawelcome::on_buttonSearx_clicked()
{
    QDesktopServices::openUrl(QUrl("https://searx.garudalinux.org/"));
}

void garudawelcome::on_buttonWiki_clicked()
{
    QDesktopServices::openUrl(QUrl("https://wiki.garudalinux.org/"));
}

void garudawelcome::on_buttonElement_clicked()
{
    QDesktopServices::openUrl(QUrl("https://element.garudalinux.org/"));
}

void garudawelcome::on_buttonfaq_clicked()
{
    QDesktopServices::openUrl(QUrl("https://forum.garudalinux.org/t/garuda-linux-faq/318/"));
}

void garudawelcome::on_buttonDonate_clicked()
{
    QDesktopServices::openUrl(QUrl("https://garudalinux.org/donate.html"));
}

void garudawelcome::on_buttonIrc_clicked()
{
    QDesktopServices::openUrl(QUrl("https://irc.garudalinux.org/"));
}

void garudawelcome::on_buttonDiscord_clicked()
{
    QDesktopServices::openUrl(QUrl("https://garudalinux.org/discord"));
}

void garudawelcome::on_buttonGarudaSettings_clicked()
{
    if (!checkAndInstall("garuda-settings-manager"))
        return;
    this->hide();
    system("garuda-settings-manager");
    this->show();
}

void garudawelcome::on_buttonGarudaAssistant_clicked()
{
    if (!checkAndInstall("garuda-assistant"))
        return;
    this->hide();
    system("garuda-assistant");
    this->show();
}

void garudawelcome::on_buttonGarudaGamer_clicked()
{
    if (!checkAndInstall("garuda-gamer"))
        return;
    this->hide();
    system("garuda-gamer");
    this->show();
}

void garudawelcome::on_buttonBootOptions_clicked()
{
    if (!checkAndInstall("garuda-boot-options"))
        return;
    this->hide();
    system("/usr/lib/garuda/pkexec-gui garuda-boot-options");
    this->show();
}

void garudawelcome::on_buttonBootRepair_clicked()
{
    if (!checkAndInstall("garuda-boot-repair"))
        return;
    this->hide();
    system("/usr/lib/garuda/pkexec-gui garuda-boot-repair");
    this->show();
}

void garudawelcome::on_buttonNetworkAssistant_clicked()
{
    if (!checkAndInstall("garuda-network-assistant"))
        return;
    this->hide();
    system("/usr/lib/garuda/pkexec-gui garuda-network-assistant");
    this->show();
}

void garudawelcome::on_buttonTimeshift_clicked()
{
    if (!checkAndInstall("btrfs-assistant"))
        return;
    this->hide();
    system("/usr/lib/garuda/pkexec-gui btrfs-assistant");
    this->show();
}

void garudawelcome::on_buttonPartition_clicked()
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

void garudawelcome::on_buttonSystemCleaner_clicked()
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

void garudawelcome::on_buttonSoftware_clicked()
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

void garudawelcome::on_buttonInstallGaruda_clicked()
{
    this->hide();
    if (QFile::exists("/usr/bin/kvantummanager")) {
        system("/usr/bin/calamares_polkit %f -style kvantum");
    } else {
        system("sudo -E calamares");
    }
    this->show();
}

void garudawelcome::on_buttonChroot_clicked()
{
    this->hide();
    system("/usr/lib/garuda/launch-terminal \"pkexec garuda-chroot -a; read -p 'Press enter to exit'\"");
    this->show();
}

void garudawelcome::on_pushButton_setupassistant_clicked()
{
    if (!checkAndInstall("garuda-setup-assistant"))
        return;
    this->hide();
    system("setup-assistant");
    this->show();
}

bool garudawelcome::checkAndInstall(QString package)
{
    // Not async, don't really care either tho, not my problem
    // Assumption: package is a safe string
    auto output = runCmd("pacman -Qq " + package, false);

    // If it's already installed, we are good to go
    if (output.output == package || output.output == package + "-git")
        return true;

    this->hide();
    runCmd("/usr/lib/garuda/launch-terminal \"/usr/lib/garuda/install-software " + package + "; read -p 'Press enter to exit'\"");

    // Checking pamac-installer's exit code alone is not enough. For some reason it decides to return 0 even tho it failed sometimes
    output = runCmd("pacman -Qq " + package, false);
    if (output.output == package)
        return true;
    else {
        this->show();
        return false;
    }
}
