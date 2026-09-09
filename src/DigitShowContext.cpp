/*
 * DigitShowBasicTS - Hollow Torsional Shear Triaxial Test Control Software
 * Copyright (C) 2026 Makoto KUNO
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "DigitShowContext.h"

static DigitShowContext g_Context;
static bool g_ContextInitialized = false;

DigitShowContext* GetContext()
{
	if (!g_ContextInitialized) {
		InitContext(&g_Context);
		g_ContextInitialized = true;
	}
	return &g_Context;
}

void InitContext(DigitShowContext* ctx)
{
	if (ctx == NULL) return;

	int i, j;
	ctx->Ret = 0;
	memset(ctx->ErrorString, 0, sizeof(ctx->ErrorString));
	ctx->TextString = _T("");
	ctx->ComPort = _T("");

	for (i = 0; i < AI_MAX_CHANNELS; i++) {
		ctx->ai.raw[i] = 0;
		ctx->ai.phy[i] = 0.0;
		ctx->ai.cal.a[i] = 0.0;
		ctx->ai.cal.b[i] = 1.0;
		ctx->ai.cal.c[i] = 0.0;
	}
	for (i = 0; i < PARAM_MAX; i++) {
		ctx->ai.param[i] = 0.0;
	}
	for (i = 0; i < AO_MAX_CHANNELS; i++) {
		ctx->ao.raw[i] = 0.0f;
		ctx->ao.cal.a[i] = 0.0;
		ctx->ao.cal.b[i] = 0.0;
	}

	ctx->NameV[0] = _T("V.Load");	ctx->NameP[0] = _T("V.Load,N");
	ctx->NameV[1] = _T("V.Disp");	ctx->NameP[1] = _T("V.DispEXT,mm");
	ctx->NameV[2] = _T("LDT1");	ctx->NameP[2] = _T("LDT1,mm");
	ctx->NameV[3] = _T("LDT2");	ctx->NameP[3] = _T("LDT2,mm");
	ctx->NameV[4] = _T("T.Load");	ctx->NameP[4] = _T("Torque,Ncm");
	ctx->NameV[5] = _T("CG1");	ctx->NameP[5] = _T("CG1,mm");
	ctx->NameV[6] = _T("CG2");	ctx->NameP[6] = _T("CG2,mm");
	ctx->NameV[7] = _T("CG3");	ctx->NameP[7] = _T("CG3,mm");
	ctx->NameV[8] = _T("HCDPT");	ctx->NameP[8] = _T("EffectiveStress,kPa");
	ctx->NameV[9] = _T("LCDPT");	ctx->NameP[9] = _T("DeltaVol.,mm3");
	ctx->NameV[10] = _T("T.Disp");	ctx->NameP[10] = _T("T.Disp,rad");
	for (i = 11; i < AI_MAX_CHANNELS; i++) {
		CString tmp;
		tmp.Format(_T("CH%d"), i);
		ctx->NameV[i] = tmp;
		ctx->NameP[i] = tmp;
	}

	ctx->NameDV[0] = _T("CH00: Axial Motor ON/OFF");
	ctx->NameDV[1] = _T("CH01: Axial Motor UP/DOWN");
	ctx->NameDV[2] = _T("CH02: Axial Motor Speed");
	ctx->NameDV[3] = _T("CH03: EP Cell Pressure");
	ctx->NameDV[4] = _T("CH04: EP Axis Pressure");
	ctx->NameDV[5] = _T("CH05: Torsional Motor ON/OFF");
	ctx->NameDV[6] = _T("CH06: Torsional Motor CW/CCW");
	ctx->NameDV[7] = _T("CH07: Torsional Motor Speed");

	memset(&ctx->phys, 0, sizeof(ctx->phys));

	for (j = 0; j < 4; j++) {
		ctx->specimen.DiameterIn[j] = 60.0;
		ctx->specimen.DiameterOut[j] = 100.0;
		ctx->specimen.Height[j] = 150.0;
		ctx->specimen.Volume[j] = 3.141592 * (100.0 * 100.0 - 60.0 * 60.0) / 4 * 150.0;
		ctx->specimen.DiaInMembrane[j] = 60.0;
		ctx->specimen.DiaOutMembrane[j] = 100.0;
		ctx->specimen.HeightInMembrane[j] = 150.0;
		ctx->specimen.HeightOutMembrane[j] = 150.0;
	}
	ctx->specimen.MembraneModulus = 1400.0;
	ctx->specimen.MembraneThickness = 0.3;
	ctx->specimen.RDiaInM = 59.85;
	ctx->specimen.RDiaOutM = 100.15;
	ctx->specimen.RHeightInM = 150.0;
	ctx->specimen.RHeightOutM = 150.0;
	ctx->specimen.RodArea = 0.0;
	ctx->specimen.CapWeight = 0.0;

	for (i = 0; i < CONTROL_MAX; i++) {
		for (j = 0; j < 3; j++) {
			ctx->control[i].flag[j] = FALSE;
			ctx->control[i].time[j] = 0;
			ctx->control[i].p[j] = 0.0;
			ctx->control[i].q[j] = 0.0;
			ctx->control[i].u[j] = 0.0;
			ctx->control[i].sigma[j] = 0.0;
			ctx->control[i].sigmaRate[j] = 0.0;
			ctx->control[i].sigmaAmp[j] = 0.0;
			ctx->control[i].strain[j] = 0.0;
			ctx->control[i].strainRate[j] = 0.0;
			ctx->control[i].strainAmp[j] = 0.0;
		}
		ctx->control[i].K0 = 1.0;
		ctx->control[i].AxisSpeed = 0.0;
		ctx->control[i].TorsionSpeed = 0.0;
	}
	ctx->control[1].q[0] = 1.0;
	ctx->control[1].AxisSpeed = 100.0;

	ctx->daCh.AxisMotor = DA_CH_AXIS_MOTOR;
	ctx->daCh.AxisDirection = DA_CH_AXIS_DIRECTION;
	ctx->daCh.AxisSpeed = DA_CH_AXIS_SPEED;
	ctx->daCh.EP_Cell = DA_CH_EP_CELL;
	ctx->daCh.EP_Axis = DA_CH_EP_AXIS;
	ctx->daCh.TorsionMotor = DA_CH_TORSION_MOTOR;
	ctx->daCh.TorsionDirection = DA_CH_TORSION_DIRECTION;
	ctx->daCh.TorsionSpeed = DA_CH_TORSION_SPEED;

	ctx->volt.Down = 0.0f;
	ctx->volt.Up = 5.0f;
	ctx->volt.CW = 0.0f;
	ctx->volt.CCW = 5.0f;

	ctx->ao.cal.a[DA_CH_AXIS_SPEED] = 0.0033333;
	ctx->ao.cal.b[DA_CH_AXIS_SPEED] = 0.0;
	ctx->ao.cal.a[DA_CH_TORSION_SPEED] = 0.0034483;
	ctx->ao.cal.b[DA_CH_TORSION_SPEED] = 0.0;
	ctx->ao.cal.a[DA_CH_EP_CELL] = 0.0175;
	ctx->ao.cal.b[DA_CH_EP_CELL] = 0.0;
	ctx->ao.cal.a[DA_CH_EP_AXIS] = ctx->ao.cal.a[DA_CH_EP_CELL];
	ctx->ao.cal.b[DA_CH_EP_AXIS] = ctx->ao.cal.b[DA_CH_EP_CELL];

	ctx->ControlID = 0;
	ctx->NumCyclic = 0;
	ctx->NumSmallCyclic = 0;
	ctx->StepTime = 0.0;
	ctx->StepDisplay = 0;
	ctx->AmpID = 1;

	ctx->target.sz = 0.0;
	ctx->target.sr = 0.0;
	ctx->target.tzq = 0.0;
	ctx->target.ez = 0.0;
	ctx->target.gzq = 0.0;

	ctx->err.StressMotor = 0.5;
	ctx->err.StressAir = 0.5;
	ctx->err.StrainEz = 0.0005;
	ctx->err.StrainGzq = 0.0005;

	ctx->flags.SetBoard = FALSE;
	ctx->flags.SaveData = FALSE;
	ctx->flags.Cyclic = FALSE;

	ctx->timeSettings.Interval1 = 200;
	ctx->timeSettings.Interval2 = 500;
	ctx->timeSettings.Interval3 = 1000;
	ctx->SequentTime1 = 0;
	ctx->SequentTime2 = 0.0;
	ctx->CtrlStepTime = 0.0;

	ctx->FileSaveData0 = NULL;
	ctx->FileSaveData1 = NULL;
	ctx->FileSaveData2 = NULL;

	ctx->controlFile.CurrentNum = 0;
	for (i = 0; i < CONTROLFILE_STEPS; i++) {
		ctx->controlFile.Num[i] = 0;
		for (j = 0; j < CONTROLFILE_PARAS; j++) {
			ctx->controlFile.Para[i][j] = 0.0;
		}
	}
}