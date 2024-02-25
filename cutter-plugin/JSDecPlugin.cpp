// SPDX-FileCopyrightText: 2024 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

#include "JSDecDecompiler.h"
#include "JSDecPlugin.h"

void JSDecPlugin::setupPlugin()
{
}

void JSDecPlugin::setupInterface(MainWindow *)
{
}

void JSDecPlugin::registerDecompilers()
{
	Core()->registerDecompiler(new JSDecDecompiler(Core()));
}
