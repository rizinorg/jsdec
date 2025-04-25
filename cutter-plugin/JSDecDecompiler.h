// SPDX-FileCopyrightText: 2024 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef JSDEC_DECOMPILER_H
#define JSDEC_DECOMPILER_H

#include "Decompiler.h"
#include "RizinTask.h"

class JSDecDecompiler: public Decompiler
{
	private:
		RizinFunctionTask *task;

	public:
		JSDecDecompiler(CutterCore *core = nullptr);
		virtual void decompileAt(RVA addr) override;
		virtual bool isRunning() override { return task != nullptr; }
};

#endif // JSDEC_DECOMPILER_H
