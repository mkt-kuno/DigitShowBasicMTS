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

#ifndef __DIGITSHOWCONTEXT_H_INCLUDE__
#define __DIGITSHOWCONTEXT_H_INCLUDE__

#pragma once

#include <afxwin.h>
#include <stdio.h>
#include <stdint.h>

#define AI_MAX_CHANNELS     16
#define AO_MAX_CHANNELS      8
#define PARAM_MAX           24
#define CONTROL_MAX         16
#define CONTROLFILE_STEPS  256
#define CONTROLFILE_PARAS   16
#define NAME_DV_MAX          8

#define DA_CH_AXIS_MOTOR         0
#define DA_CH_AXIS_DIRECTION     1
#define DA_CH_AXIS_SPEED         2
#define DA_CH_EP_CELL            3
#define DA_CH_EP_AXIS            4
#define DA_CH_TORSION_MOTOR      5
#define DA_CH_TORSION_DIRECTION  6
#define DA_CH_TORSION_SPEED      7

struct SpecimenData {
	double DiameterIn[4];       double DiameterOut[4];
	double Height[4];
	double Volume[4];
	double MembraneModulus;     double MembraneThickness;
	double RodArea;             double CapWeight;
	double RDiaInM;             double RDiaOutM;
	double RHeightInM;          double RHeightOutM;
	double DiaInMembrane[4];    double DiaOutMembrane[4];
	double HeightInMembrane[4]; double HeightOutMembrane[4];
};

struct ControlData {
	bool   flag[3];
	int    time[3];
	double p[3];
	double q[3];
	double u[3];
	double sigma[3];
	double sigmaRate[3];
	double sigmaAmp[3];
	double strain[3];
	double strainRate[3];
	double strainAmp[3];
	double K0;
	double AxisSpeed;
	double TorsionSpeed;
};

struct PhysicalValues {
	double rotation1;
	double rotation2;
	double BW2;
	double height;
	double area;
	double volume;
	double diameter_in;
	double diameter_out;
	double diameterInM;
	double diameterOutM;
	double heightInM;
	double heightOutM;
	double cell_in;
	double cell_out;
	double sz;
	double sr;
	double sq;
	double szq;
	double p;
	double q;
	double ez;
	double er;
	double eq;
	double gzq1;
	double gzq2;
	double ev;
	double ezInM;
	double ezOutM;
	double eqInM;
	double eqOutM;
	double gzqInM;
	double gzqOutM;
	double PressureInM;
	double PressureOutM;
	double ForceM;
	double TorqueM;
};

struct ControlFileData {
	int    CurrentNum;
	int    Num[CONTROLFILE_STEPS];
	double Para[CONTROLFILE_STEPS][CONTROLFILE_PARAS];
};

struct TimeSettings {
	unsigned int Interval1;
	unsigned int Interval2;
	unsigned int Interval3;
};

struct SystemFlags {
	bool SetBoard;
	bool SaveData;
	bool Cyclic;
};

struct DigitShowContext {
	long   Ret;
	char   ErrorString[256];
	CString TextString;
	CString ComPort;

	CString NameV[AI_MAX_CHANNELS];
	CString NameP[AI_MAX_CHANNELS];
	CString NameDV[NAME_DV_MAX];

	struct {
		int16_t raw[AI_MAX_CHANNELS];
		double phy[AI_MAX_CHANNELS];
		double param[PARAM_MAX];
		struct {
			double a[AI_MAX_CHANNELS];
			double b[AI_MAX_CHANNELS];
			double c[AI_MAX_CHANNELS];
		} cal;
	} ai;

	struct {
		float  raw[AO_MAX_CHANNELS];
		struct {
			double a[AO_MAX_CHANNELS];
			double b[AO_MAX_CHANNELS];
		} cal;
	} ao;

	PhysicalValues phys;
	SpecimenData specimen;
	ControlData control[CONTROL_MAX];
	ControlFileData controlFile;

	struct {
		int AxisMotor;
		int AxisDirection;
		int AxisSpeed;
		int EP_Cell;
		int EP_Axis;
		int TorsionMotor;
		int TorsionDirection;
		int TorsionSpeed;
	} daCh;

	struct {
		float Up;
		float Down;
		float CW;
		float CCW;
	} volt;

	int    ControlID;
	int    NumCyclic;
	int    NumSmallCyclic;
	double StepTime;
	int    StepDisplay;
	int    AmpID;

	struct {
		double sz;
		double sr;
		double tzq;
		double ez;
		double gzq;
	} target;

	struct {
		double StressMotor;
		double StressAir;
		double StrainEz;
		double StrainGzq;
	} err;

	SystemFlags flags;
	TimeSettings timeSettings;
	CTime        StartTime;
	CTime        NowTime;
	CTimeSpan    SpanTime;
	CString      SNowTime;
	long         SequentTime1;
	double       SequentTime2;
	double       CtrlStepTime;

	FILE* FileSaveData0;
	FILE* FileSaveData1;
	FILE* FileSaveData2;
};

DigitShowContext* GetContext();
void InitContext(DigitShowContext* ctx);

#endif // __DIGITSHOWCONTEXT_H_INCLUDE__