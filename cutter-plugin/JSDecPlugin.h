// SPDX-FileCopyrightText: 2024 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef JSDEC_CUTTER_PLUGIN_H
#define JSDEC_CUTTER_PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include <plugins/CutterPlugin.h>

class JSDecPlugin : public QObject, CutterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "re.rizin.cutter.plugins.jsdec")
    Q_INTERFACES(CutterPlugin)

public:
    void setupPlugin() override;
    void setupInterface(MainWindow *main) override;
    void registerDecompilers() override;

    QString getName() const override        { return "JsDec Decompiler (jsdec)"; }
    QString getAuthor() const override      { return "deroad"; }
    QString getDescription() const override { return "GUI Integration of jsdec."; }
    QString getVersion() const override     { return "1.0"; }
};


#endif // JSDEC_CUTTER_PLUGIN_H
