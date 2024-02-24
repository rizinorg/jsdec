// SPDX-FileCopyrightText: 2024 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#include "JSDecDecompiler.h"
#include <Cutter.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "../c/jsdec-cutter.h"

JSDecDecompiler::JSDecDecompiler(QObject *parent)
    : Decompiler("jsdec", "jsdec", parent) {
	task = nullptr;
}

void JSDecDecompiler::decompileAt(RVA addr) {
	if (task) {
		return;
	}

	task = new RizinFunctionTask([addr](RzCore *core) {
		return jsdec_as_annotation(core, addr);
	});

	connect(task, &RizinFunctionTask::finished, this, [this]() {
		auto res = reinterpret_cast<RzAnnotatedCode *>(task->getResult());
        delete task;
        task = nullptr;
		if (!res) {
			res = Decompiler::makeWarning(tr("Failed to parse JSON from jsdec"));
		}
		emit finished(res);
	});

	task->startTask();
}
