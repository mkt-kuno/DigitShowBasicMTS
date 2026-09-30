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
// DigitShowBasicDoc.cpp : CDigitShowBasicDoc クラスの動作の定義を行います。
//

#include	"stdafx.h"
#include	"DigitShowBasic.h"
#include	"DigitShowBasicDoc.h"
#include	"DigitShowContext.h"
#include	"ModbusRTU.h"

#include	<setupapi.h>
#pragma comment(lib, "setupapi.lib")

#include	"time.h"
#include	"math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDigitShowBasicDoc


IMPLEMENT_DYNCREATE(CDigitShowBasicDoc, CDocument)

BEGIN_MESSAGE_MAP(CDigitShowBasicDoc, CDocument)
	//{{AFX_MSG_MAP(CDigitShowBasicDoc)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDigitShowBasicDoc クラスの構築/消滅

CDigitShowBasicDoc::CDigitShowBasicDoc()
{
	DigitShowContext* ctx = GetContext();
}

CDigitShowBasicDoc::~CDigitShowBasicDoc()
{
}

BOOL CDigitShowBasicDoc::OnNewDocument()
{	DigitShowContext* ctx = GetContext();
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDigitShowBasicDoc シリアライゼーション

void CDigitShowBasicDoc::Serialize(CArchive& ar)
{	DigitShowContext* ctx = GetContext();
	if (ar.IsStoring())
	{
		// TODO: この位置に保存用のコードを追加してください。
	}
	else
	{
		// TODO: この位置に読み込み用のコードを追加してください。
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDigitShowBasicDoc クラスの診断

#ifdef _DEBUG
void CDigitShowBasicDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDigitShowBasicDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

static CString DetectArduinoPort()
{
	static const struct { const char* vid; const char* pid; int priority; } knownDevices[] = {
		{ "2341", "0069", 95 }, { "2341", "0074", 95 }, { "2341", "0243", 95 }, { "2341", NULL, 90 },
		{ "2E8A", "000A", 90 }, { "2E8A", "0005", 88 }, { "2E8A", NULL, 86 },
		{ "0483", "5740", 85 }, { "0483", "374B", 82 },
		{ "1A86", "7523", 80 }, { "1A86", "55D3", 80 }, { "1A86", "7522", 80 }, { "1A86", NULL, 78 },
		{ "10C4", "EA60", 80 }, { "10C4", NULL, 78 },
		{ "0403", "6001", 80 }, { "0403", "6015", 80 }, { "0403", NULL, 78 },
	};
	const int numKnown = static_cast<int>(sizeof(knownDevices) / sizeof(knownDevices[0]));
	CString bestPort = _T("");
	int bestPriority = -1;
	int bestPortNum = -1;
	static const GUID GUID_DEVCLASS_PORTS = { 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE) return bestPort;
	SP_DEVINFO_DATA devInfoData; devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (DWORD idx = 0; SetupDiEnumDeviceInfo(hDevInfo, idx, &devInfoData); idx++) {
		char hwId[1024] = { 0 };
		if (!SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_HARDWAREID, NULL, reinterpret_cast<PBYTE>(hwId), sizeof(hwId) - 1, NULL)) continue;
		bool isUSB = (_strnicmp(hwId, "USB\\", 4) == 0);
		bool isFTDI = (_strnicmp(hwId, "FTDIBUS\\", 8) == 0);
		if (!isUSB && !isFTDI) continue;
		char friendlyName[256] = { 0 };
		if (!SetupDiGetDeviceRegistryPropertyA(hDevInfo, &devInfoData, SPDRP_FRIENDLYNAME, NULL, reinterpret_cast<PBYTE>(friendlyName), sizeof(friendlyName) - 1, NULL)) continue;
		char* comStart = strstr(friendlyName, "(COM");
		if (!comStart) continue;
		char portNumStr[16] = { 0 };
		if (sscanf_s(comStart + 4, "%10[^)]", portNumStr, static_cast<unsigned int>(sizeof(portNumStr))) != 1) continue;
		int portNum = atoi(portNumStr);
		CString portName; portName.Format(_T("COM%s"), portNumStr);
		char* vidPtr = strstr(hwId, "VID_"); char* pidPtr = strstr(hwId, "PID_");
		if (!vidPtr || !pidPtr) continue;
		char vid[5] = { 0 }, pid[5] = { 0 };
		strncpy_s(vid, sizeof(vid), vidPtr + 4, _TRUNCATE); strncpy_s(pid, sizeof(pid), pidPtr + 4, _TRUNCATE);
		_strupr_s(vid); _strupr_s(pid);
		int priority = 1;
		for (int k = 0; k < numKnown; k++) if (_stricmp(vid, knownDevices[k].vid) == 0 && (knownDevices[k].pid == NULL || _stricmp(pid, knownDevices[k].pid) == 0)) { priority = knownDevices[k].priority; break; }
		if (priority > bestPriority || (priority == bestPriority && portNum > bestPortNum)) { bestPriority = priority; bestPortNum = portNum; bestPort = portName; }
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return bestPort;
}

/////////////////////////////////////////////////////////////////////////////
// CDigitShowBasicDoc コマンド
void CDigitShowBasicDoc::OpenBoard()
{	DigitShowContext* ctx = GetContext();
	ModbusRTU* modbus = GetModbusInstance();
	if (ctx->flags.SetBoard) {
		AfxMessageBox("Initialization has been already accomplished", MB_ICONSTOP | MB_OK);
		return;
	}
	CString strPort = DetectArduinoPort();
	if (strPort.IsEmpty()) {
		AfxMessageBox("No Arduino-compatible USB COM port found.\nCheck device connection and driver.", MB_ICONSTOP | MB_OK);
		return;
	}
	ctx->ComPort = strPort;
	if (!modbus->Open(CT2A(strPort), 1)) {
		ctx->TextString.Format("Modbus Open failed: %s", modbus->GetLastError());
		AfxMessageBox(ctx->TextString, MB_ICONSTOP | MB_OK);
		return;
	}
	for (int i = 0; i < AO_MAX_CHANNELS; i++) ctx->ao.raw[i] = 0.0f;
	uint16_t zeroOutput[ModbusRTU::AO_CHANNELS] = { 0 };
	if (!modbus->WriteHoldingRegisters(zeroOutput, true)) {
		ctx->TextString.Format("Initial output reset failed: %s", modbus->GetLastError());
		AfxMessageBox(ctx->TextString, MB_ICONSTOP | MB_OK);
		modbus->Close();
		ctx->ComPort = _T("");
		return;
	}
	ctx->flags.SetBoard = TRUE;
}

void CDigitShowBasicDoc::CloseBoard()
{	DigitShowContext* ctx = GetContext();
	if(ctx->flags.SetBoard){
		ModbusRTU* modbus = GetModbusInstance();
		for (int i = 0; i < AO_MAX_CHANNELS; i++) ctx->ao.raw[i] = 0.0f;
		uint16_t zeroOutput[ModbusRTU::AO_CHANNELS] = { 0 };
		modbus->WriteHoldingRegisters(zeroOutput, true);
		modbus->Close();
		ctx->flags.SetBoard=FALSE;
		ctx->ComPort = _T("");
	}
}

//--- Input from A/D Board ---
void CDigitShowBasicDoc::AD_INPUT()
{	DigitShowContext* ctx = GetContext();
	ModbusRTU* modbus = GetModbusInstance();
	if(!modbus->IsOpen()) return;
	int16_t aiData[ModbusRTU::AI_CHANNELS];
	if(!modbus->ReadInputRegisters(aiData)){
		ctx->TextString.Format("Modbus read failed: %s", modbus->GetLastError());
		return;
	}
	for(int i=0;i<ModbusRTU::AI_CHANNELS;i++) ctx->ai.raw[i] = aiData[i];
}
//--- Output to D/A Board ---
void CDigitShowBasicDoc::DA_OUTPUT()
{	DigitShowContext* ctx = GetContext();
	ModbusRTU* modbus = GetModbusInstance();
	if(!modbus->IsOpen()) return;
	uint16_t aoData[ModbusRTU::AO_CHANNELS];
	for(int i=0;i<ModbusRTU::AO_CHANNELS;i++){
		if(ctx->ao.raw[i] < 0.0f) ctx->ao.raw[i] = 0.0f;
		if(ctx->ao.raw[i] > 10.0f) ctx->ao.raw[i] = 10.0f;
		aoData[i] = static_cast<uint16_t>(ctx->ao.raw[i] * 1000.0f + 0.5f);
	}
	if(!modbus->WriteHoldingRegisters(aoData)) ctx->TextString.Format("Modbus write failed: %s", modbus->GetLastError());
}
//--- Calcuration of Physical Value ---
void CDigitShowBasicDoc::Cal_Physical()
{	DigitShowContext* ctx = GetContext();
	for(int i=0;i<AI_MAX_CHANNELS;i++){
		double rawValue = static_cast<double>(ctx->ai.raw[i]);
		ctx->ai.phy[i]=ctx->ai.cal.a[i]*rawValue*rawValue + ctx->ai.cal.b[i]*rawValue + ctx->ai.cal.c[i];
	}
}

//--- Calcuration of the Other Parameters ---
void CDigitShowBasicDoc::Cal_Param()
{	DigitShowContext* ctx = GetContext();
	ctx->phys.BW2 = ctx->ai.phy[9];
	ctx->phys.height=ctx->specimen.Height[0]-ctx->ai.phy[1];
	ctx->phys.volume=ctx->specimen.Volume[0]-ctx->phys.BW2;
	ctx->phys.area=ctx->phys.volume/ctx->phys.height;
	ctx->phys.rotation1 = ctx->ai.phy[10] * 3.14159265358979323846 / 180.0;
	ctx->phys.rotation2 = ctx->phys.rotation1; // Consolidated from dual POT to single Tor disp (deg), converted to rad.
	ctx->phys.diameter_in=ctx->specimen.DiameterIn[0]*sqrt((1-ctx->phys.BW2/ctx->specimen.Volume[0])/(1-ctx->ai.phy[1]/ctx->specimen.Height[0]));
	ctx->phys.diameter_out=ctx->specimen.DiameterOut[0]*sqrt((1-ctx->phys.BW2/ctx->specimen.Volume[0])/(1-ctx->ai.phy[1]/ctx->specimen.Height[0]));
	ctx->phys.diameterInM = ctx->phys.diameter_in - ctx->specimen.MembraneThickness / 2.0;
	ctx->phys.diameterOutM=ctx->phys.diameter_out+ctx->specimen.MembraneThickness/2.0;
	ctx->phys.heightInM=ctx->specimen.HeightInMembrane[0]-ctx->ai.phy[1];
	ctx->phys.heightOutM=ctx->specimen.HeightOutMembrane[0]-ctx->ai.phy[1];
	ctx->phys.ez=ctx->ai.phy[1]/ctx->specimen.Height[0];
	ctx->phys.er=-((ctx->phys.diameter_out-ctx->specimen.DiameterOut[0])-(ctx->phys.diameter_in-ctx->specimen.DiameterIn[0]))/(ctx->phys.diameter_out-ctx->phys.diameter_in);
	ctx->phys.eq=-((ctx->phys.diameter_out-ctx->specimen.DiameterOut[0])+(ctx->phys.diameter_in-ctx->specimen.DiameterIn[0]))/(ctx->phys.diameter_out+ctx->phys.diameter_in);
	ctx->phys.gzq1=ctx->phys.rotation1*(pow(ctx->phys.diameter_out,3.0)-pow(ctx->phys.diameter_in, 3.0))/3.0/ctx->phys.height/(pow(ctx->phys.diameter_out, 2.0)-pow(ctx->phys.diameter_in, 2.0));
	ctx->phys.gzq2=ctx->phys.rotation2*(pow(ctx->phys.diameter_out,3.0)-pow(ctx->phys.diameter_in, 3.0))/3.0/ctx->phys.height/(pow(ctx->phys.diameter_out, 2.0)-pow(ctx->phys.diameter_in, 2.0));
	ctx->phys.ev=ctx->phys.BW2/ctx->specimen.Volume[0];
	ctx->phys.ezInM=(ctx->specimen.RHeightInM-ctx->phys.heightInM)/ctx->specimen.RHeightInM;
	ctx->phys.ezOutM=(ctx->specimen.RHeightOutM-ctx->phys.heightOutM)/ctx->specimen.RHeightOutM;
	ctx->phys.eqInM=(ctx->specimen.RDiaInM-ctx->phys.diameterInM)/ctx->specimen.RDiaInM;
	ctx->phys.eqOutM=(ctx->specimen.RDiaOutM-ctx->phys.diameterOutM)/ctx->specimen.RDiaOutM;
	ctx->phys.gzqInM = ctx->phys.diameterInM / 2.0 * ctx->phys.rotation1 / ctx->specimen.RHeightInM;
	ctx->phys.gzqOutM = ctx->phys.diameterOutM / 2.0 * ctx->phys.rotation1 / ctx->specimen.RHeightOutM;
	ctx->phys.TorqueM=-1.0/6.0*3.141592*ctx->specimen.MembraneModulus*ctx->specimen.MembraneThickness*(pow(ctx->phys.diameter_in, 2.0)*ctx->phys.gzqInM+pow(ctx->phys.diameter_out, 2.0)*ctx->phys.gzqOutM)/1000000.0;
	ctx->phys.PressureInM = 0.0;
	ctx->phys.PressureOutM = 0.0;
	ctx->phys.ForceM = 0.0;
	ctx->ai.phy[0]=ctx->ai.phy[0]+ctx->phys.ForceM+ctx->specimen.CapWeight;
	ctx->ai.phy[4]=ctx->ai.phy[4]+ctx->phys.TorqueM*100.0;
	ctx->phys.cell_out=ctx->ai.phy[8]+ctx->phys.PressureOutM;
	ctx->phys.cell_in=ctx->ai.phy[8]+ctx->phys.PressureInM;
	ctx->phys.sz=(ctx->ai.phy[0]+3.141592/4.0*(ctx->phys.cell_out*pow(ctx->phys.diameter_out, 2.0)-ctx->phys.cell_in*pow(ctx->phys.diameter_in, 2.0))/1000.0)/ctx->phys.area*1000.0;
	ctx->phys.sr=(ctx->phys.cell_out*ctx->phys.diameter_out+ctx->phys.cell_in*ctx->phys.diameter_in)/(ctx->phys.diameter_out+ctx->phys.diameter_in);
	ctx->phys.sq=(ctx->phys.cell_out*ctx->phys.diameter_out-ctx->phys.cell_in*ctx->phys.diameter_in)/(ctx->phys.diameter_out-ctx->phys.diameter_in);
	ctx->phys.szq=4.0*(ctx->ai.phy[4]/100.0)/3.141592*(3.0/2.0/(pow(ctx->phys.diameter_out, 3.0)-pow(ctx->phys.diameter_in, 3.0))+1.0/(pow(ctx->phys.diameter_out, 2.0)+pow(ctx->phys.diameter_in, 2.0))/(ctx->phys.diameter_out-ctx->phys.diameter_in))*1000000.0;
	ctx->phys.p=(ctx->phys.sz+ctx->phys.sr+ctx->phys.sq)/3.0;
	ctx->phys.q=ctx->phys.sz-ctx->phys.sr;
	ctx->ai.param[0]=ctx->phys.sz; ctx->ai.param[1]=ctx->phys.sr; ctx->ai.param[2]=ctx->phys.sq; ctx->ai.param[3]=ctx->phys.szq;
	ctx->ai.param[4]=ctx->phys.ev*100.0; ctx->ai.param[5]=ctx->phys.ez*100.0; ctx->ai.param[6]=ctx->ai.phy[2]; ctx->ai.param[7]=ctx->ai.phy[3];
	ctx->ai.param[8]=ctx->ai.phy[5]; ctx->ai.param[9]=ctx->ai.phy[6]; ctx->ai.param[10]=ctx->ai.phy[7]; ctx->ai.param[11]=ctx->phys.p;
	ctx->ai.param[12]=ctx->phys.q;
	ctx->ai.param[13]=(ctx->phys.sz + ctx->phys.sq)/2.0 + sqrt((ctx->phys.sz - ctx->phys.sq)*(ctx->phys.sz - ctx->phys.sq)/4 + ctx->phys.szq*ctx->phys.szq);
	ctx->ai.param[14]=ctx->phys.sr;
	ctx->ai.param[15]=(ctx->phys.sz + ctx->phys.sq)/2.0 - sqrt((ctx->phys.sz - ctx->phys.sq)*(ctx->phys.sz - ctx->phys.sq)/4 + ctx->phys.szq*ctx->phys.szq);
	ctx->ai.param[16]=ctx->phys.gzq1*100.0; ctx->ai.param[17]=ctx->phys.gzq2*100.0; ctx->ai.param[18]=ctx->phys.cell_in; ctx->ai.param[19]=ctx->phys.cell_out;
	ctx->ai.param[20]=ctx->phys.diameter_in; ctx->ai.param[21]=ctx->phys.diameter_out; ctx->ai.param[22]=ctx->phys.height; ctx->ai.param[23]=ctx->phys.volume;
	ctx->StepDisplay = ctx->controlFile.CurrentNum;
}
//--- Save the data to File ---
void CDigitShowBasicDoc::SaveToFile()
{	DigitShowContext* ctx = GetContext();
	fprintf(ctx->FileSaveData0,"%.3lf	",ctx->SequentTime2);
	fprintf(ctx->FileSaveData1,"%.3lf	",ctx->SequentTime2);
	for(int i=0;i<AI_MAX_CHANNELS;i++){
		fprintf(ctx->FileSaveData0,"%d	",static_cast<int>(ctx->ai.raw[i]));
		fprintf(ctx->FileSaveData1,"%lf	",ctx->ai.phy[i]);
	}
	fprintf(ctx->FileSaveData0,"\n");
	fprintf(ctx->FileSaveData1,"\n");
	fprintf(ctx->FileSaveData2,"%.3lf	",ctx->SequentTime2);
	for(int i=0;i<PARAM_MAX;i++) fprintf(ctx->FileSaveData2,"%lf	",ctx->ai.param[i]);
	fprintf(ctx->FileSaveData2, "%d	", ctx->StepDisplay);
	fprintf(ctx->FileSaveData2, "%d	", ctx->NumCyclic);
	fprintf(ctx->FileSaveData2,"\n");
}

//--- Control Statements ---
void CDigitShowBasicDoc::Control_DA()
{	DigitShowContext* ctx = GetContext();
	switch (ctx->ControlID)
	{
	case 0:
		{ 
			Stop_Control();
		}
		break;
	case 1:
		{ 
			// Pre-Consolidation Process
			// ctx->control[1].AxisSpeed:	Maximum Speed
			// ctx->control[1].q:			Deviator stress when the motor speed become the max. value;
			// ctx->control[1].sigma[1]:		Target of Cell Pressure
			// ctx->control[1].sigmaRate[1]: Increment Rate of Cell Pressure 
			if( ctx->control[1].sigma[1] > 0.0 && ctx->control[1].sigmaRate[1] > 0.0 ){
				if(ctx->phys.sr <= ctx->control[1].sigma[1]-ctx->err.StressAir)		ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(ctx->ao.cal.a[ctx->daCh.EP_Cell]*ctx->control[1].sigmaRate[1]*ctx->timeSettings.Interval2/1000.0/60.0);
				else if(ctx->phys.sr >= ctx->control[1].sigma[1]+ctx->err.StressAir)	ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]-float(ctx->ao.cal.a[ctx->daCh.EP_Cell]*ctx->control[1].sigmaRate[1]*ctx->timeSettings.Interval2/1000.0/60.0);
				else{
					ctx->target.sr=ctx->control[1].sigma[1];
					ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(0.2*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
				}
			}
			
			// Axial control
			ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;			// Motor: On
			if( ctx->phys.q > ctx->err.StressMotor ){
				ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;	// Clutch: Unloading
				if( ctx->phys.q > ctx->control[1].q[0] )	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->control[1].AxisSpeed+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
				if( ctx->phys.q <= ctx->control[1].q[0] )	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*(ctx->phys.q/ctx->control[1].q[0])*ctx->control[1].AxisSpeed+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
			}
			else if( ctx->phys.q < -ctx->err.StressMotor ){
				ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;		// Clutch: loading
				if( ctx->phys.q < -ctx->control[1].q[0] )	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->control[1].AxisSpeed+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
				if( ctx->phys.q >= -ctx->control[1].q[0] )	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*(-ctx->phys.q/ctx->control[1].q[0])*ctx->control[1].AxisSpeed+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
			}
			else {
				ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;	// RPM->0
			}

			// Torque control @note Hashimoto modified 2022.12
			//float torsional_speed = 10.f; //10[RPM]
			//ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;			// Motor: On
			//ctx->target.tzq = 0;
			//if (ctx->phys.szq > ctx->target.tzq + ctx->err.StressMotor) {
			//	ctx->ao.raw[ctx->daCh.TorsionSpeed] = float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed] * torsional_speed + ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
			//	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
			//}
			//else if (ctx->phys.szq < ctx->target.tzq - ctx->err.StressMotor) {
			//	ctx->ao.raw[ctx->daCh.TorsionSpeed] = float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed] * torsional_speed + ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
			//	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
			//}
			//else {
			//	ctx->ao.raw[ctx->daCh.TorsionSpeed] = 0.0f;
			//}

			DA_OUTPUT();
		}
		break;
	case 2:
		{ 
			// Consolidation Process
			// ctx->control[2].sigma[0]:		Axial effective stress
			// ctx->control[2].K0:			K0 value
			// ctx->control[2].AxisSpeed		Axial motor speed (RPM)
			// ctx->control[2].sigmaRate[2]: Increment rate of cell pressure
			if( ctx->control[2].sigmaRate[2] > 0.0 ){
				if( ctx->phys.sr < ctx->control[2].sigma[0]*ctx->control[2].K0 - ctx->err.StressAir){
					ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(ctx->ao.cal.a[ctx->daCh.EP_Cell]*ctx->control[2].sigmaRate[2]/60.0*ctx->timeSettings.Interval2/1000.0);
				}	
				if( ctx->phys.sr > ctx->control[2].sigma[0]*ctx->control[2].K0 + ctx->err.StressAir){
					ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]-float(ctx->ao.cal.a[ctx->daCh.EP_Cell]*ctx->control[2].sigmaRate[2]/60.0*ctx->timeSettings.Interval2/1000.0);
				}
			}
			if( ctx->control[2].K0 != 0.0 ){
				ctx->target.sz = ctx->phys.sr / ctx->control[2].K0;
				ctx->ao.raw[ctx->daCh.AxisMotor] = 5.0f;			// Motor: On
				ctx->ao.raw[ctx->daCh.AxisSpeed] = float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->control[2].AxisSpeed+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
				if( ctx->phys.sz > ctx->target.sz + ctx->err.StressMotor )			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;	// Clutch: Unloading
				else if( ctx->phys.sz < ctx->target.sz - ctx->err.StressMotor )	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;	// Clutch: loading
				else											ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;	// RPM->0
			}		
			DA_OUTPUT();
		}
		break;
	case 3:
		{ 
			DA_OUTPUT();
		}
		break;
	case 4:
		{ 
		DA_OUTPUT();
		}
		break;
	case 5:
		{	
			DA_OUTPUT();
		}
		break;
	case 6:
		{ 
			DA_OUTPUT();
		}
		break;
	case 7:
		{ 
			DA_OUTPUT();
		}
		break;
	case 8:
		{ 
			DA_OUTPUT();
		}
		break;
	case 9:
		{ 
			DA_OUTPUT();
		}
		break;
	case 10:
		{ 
			DA_OUTPUT();
		}
		break;
	case 11:
		{ 
			DA_OUTPUT();
		}
		break;
	case 12:
		{ 
			DA_OUTPUT();
		}
		break;
	case 13:
		{ 
			DA_OUTPUT();
		}
		break;
	case 14:
		{ 
			DA_OUTPUT();
		}
		break;
	case 15:
		{ 
			if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==0 ){
				// 2023.11.29 Edited by Hashimoto
				// stop any loading if control Number is 0
				ctx->ao.raw[ctx->daCh.AxisMotor] = 0.0f;
				ctx->ao.raw[ctx->daCh.AxisSpeed] = 0.0f;
				ctx->ao.raw[ctx->daCh.TorsionMotor] = 0.0f;
				ctx->ao.raw[ctx->daCh.TorsionSpeed] = 0.0f;
			}
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==1 ) EffectiveStressPathLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==2 ) MonotonicTorsionalLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==3 ) MonotonicTorsionalLoadingCNS();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==4 ) CyclicTorsionalLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==5 ) CyclicTorsionalLoadingCNS();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==6 ) SmallCyclicTorsionalLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==7 ) SmallCyclicTorsionalLoadingCNS();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==8 ) MonotonicAxialLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==9 ) CyclicAxialLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==10) SmallCyclicAxialLoading();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==11) Creep();
			// 2021.06.07 Edited by M.Kuno
			// customize for Sanjei
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==12) MonotonicAxialLoadingConstP();
			else if( ctx->controlFile.Num[ctx->controlFile.CurrentNum]==13) MonotonicTorsionalLoadingConstPA();
			else if (ctx->controlFile.Num[ctx->controlFile.CurrentNum]==14) CyclicAxialLoading_OR();
			else if (ctx->controlFile.Num[ctx->controlFile.CurrentNum] == 15) FileControlableConsolidation();
			DA_OUTPUT();
		}
		break;
	}
}

void CDigitShowBasicDoc::Stop_Control()
{	DigitShowContext* ctx = GetContext();
	ctx->ao.raw[ctx->daCh.AxisMotor]=0.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
	ctx->ao.raw[ctx->daCh.TorsionMotor]=0.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=0.0f;
	DA_OUTPUT();
}

void CDigitShowBasicDoc::FileControlableConsolidation()
{	DigitShowContext* ctx = GetContext();
	// 0: sigma_z_ini, 
	// 1: sigma_r_ini, 
	// 2: tau_zq_ini, 
	// 3: sigma_z_end, 
	// 4: sigma_r_end, 
	// 5: tau_zq_end.
	// 6: Axial Motor Speed	(RPM)
	// 7: Torsion Motor Speed (RPM)
	// 8: Cell Pressure Rate (min/kPa)
	ctx->StepTime = ctx->StepTime + ctx->CtrlStepTime / 60.0;
	
	if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][8] > 0.0) {
		if (ctx->phys.sr < ctx->controlFile.Para[ctx->controlFile.CurrentNum][4] - ctx->err.StressAir) {
			ctx->ao.raw[ctx->daCh.EP_Cell] = ctx->ao.raw[ctx->daCh.EP_Cell] + float(ctx->ao.cal.a[ctx->daCh.EP_Cell] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][8] / 60.0 * ctx->timeSettings.Interval2 / 1000.0);
		}
		if (ctx->phys.sr > ctx->controlFile.Para[ctx->controlFile.CurrentNum][4] + ctx->err.StressAir) {
			ctx->ao.raw[ctx->daCh.EP_Cell] = ctx->ao.raw[ctx->daCh.EP_Cell] - float(ctx->ao.cal.a[ctx->daCh.EP_Cell] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][8] / 60.0 * ctx->timeSettings.Interval2 / 1000.0);
		}
	}

	if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][4] == ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) {
		ctx->controlFile.CurrentNum = ctx->controlFile.CurrentNum + 1;
		ctx->StepTime = 0.0;
	}
	else {
		float comp_rate = (ctx->phys.sr - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) / (ctx->controlFile.Para[ctx->controlFile.CurrentNum][4] - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]);
		float Target_szq = ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] + comp_rate * (ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] - ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]);
		ctx->target.sz = ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] + comp_rate * (ctx->controlFile.Para[ctx->controlFile.CurrentNum][3] - ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]);

		ctx->ao.raw[ctx->daCh.AxisMotor] = 5.0f;			// Axial Motor: On
		ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;         // Torsional Motor: On
		ctx->ao.raw[ctx->daCh.AxisSpeed] = float(ctx->ao.cal.a[ctx->daCh.AxisSpeed] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][6] + ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
		ctx->ao.raw[ctx->daCh.TorsionSpeed] = float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][7] + ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);

		if (ctx->phys.sz > ctx->target.sz + ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Up;	// Clutch: Unloading
		else if (ctx->phys.sz < ctx->target.sz - ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Down;	// Clutch: loading
		else											ctx->ao.raw[ctx->daCh.AxisSpeed] = 0.0f;	// RPM->0

		if (ctx->phys.szq > Target_szq + ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;	// Clutch: Unloading
		else if (ctx->phys.szq < Target_szq - ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;	// Clutch: loading
		else											ctx->ao.raw[ctx->daCh.TorsionSpeed] = 0.0f;	// RPM->0
	}

	if (fabs(ctx->phys.sz - ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) <= ctx->err.StressMotor * 2.0 && fabs(ctx->phys.sr - ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]) <= ctx->err.StressMotor * 2.0 && fabs(ctx->phys.szq - ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) <= ctx->err.StressMotor * 2.0) {
		ctx->controlFile.CurrentNum = ctx->controlFile.CurrentNum + 1;
		ctx->StepTime = 0.0;
	}
}


void CDigitShowBasicDoc::EffectiveStressPathLoading()
{	DigitShowContext* ctx = GetContext();
	// 0: sigma_z_ini, 
	// 1: sigma_r_ini, 
	// 2: tau_zq_ini, 
	// 3: sigma_z_end, 
	// 4: sigma_r_end, 
	// 5: tau_zq_end.
	// 6: Axial Motor Speed	(RPM)
	// 7: Torsion Motor Speed (RPM)
	// 8: Cell Pressure Rate (min/kPa),
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] != ctx->controlFile.Para[ctx->controlFile.CurrentNum][4] ){
		if(ctx->phys.sr < ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]-ctx->err.StressAir) ctx->target.sr=ctx->phys.sr+ctx->controlFile.Para[ctx->controlFile.CurrentNum][8]*ctx->timeSettings.Interval2/1000/60;
		if(ctx->phys.sr > ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]+ctx->err.StressAir) ctx->target.sr=ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][8]*ctx->timeSettings.Interval2/1000/60;
		if(fabs(ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]) <= ctx->err.StressAir) ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][4];
		ctx->target.sz=(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][0])/(ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])*(ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])+ctx->controlFile.Para[ctx->controlFile.CurrentNum][0];
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3] > ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] && ctx->target.sz > ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->target.sz=ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]; 
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3] < ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] && ctx->target.sz < ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->target.sz=ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]; 
		ctx->target.tzq=(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])/(ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])*(ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])+ctx->controlFile.Para[ctx->controlFile.CurrentNum][2];
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->target.tzq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) ctx->target.tzq=ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]; 
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->target.tzq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) ctx->target.tzq=ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]; 
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] != ctx->controlFile.Para[ctx->controlFile.CurrentNum][3] && fabs(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]) >= fabs(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])){
		ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][4];
		ctx->target.sz=ctx->controlFile.Para[ctx->controlFile.CurrentNum][3];
		ctx->target.tzq=(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])/(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][0])*(ctx->phys.sz-ctx->controlFile.Para[ctx->controlFile.CurrentNum][0])+ctx->controlFile.Para[ctx->controlFile.CurrentNum][2];
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->target.tzq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) ctx->target.tzq=ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]; 
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->target.tzq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) ctx->target.tzq=ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]; 
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] != ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){
		ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][4];
		ctx->target.sz=(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][0])/(ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]-ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])*(ctx->phys.szq-ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])+ctx->controlFile.Para[ctx->controlFile.CurrentNum][0];
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3] > ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] && ctx->target.sz > ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->target.sz=ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]; 
		if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][3] < ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] && ctx->target.sz < ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->target.sz=ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]; 
		ctx->target.tzq=ctx->controlFile.Para[ctx->controlFile.CurrentNum][5];
	}
	else {
		ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][4];
		ctx->target.sz=ctx->controlFile.Para[ctx->controlFile.CurrentNum][3];
		ctx->target.tzq=ctx->controlFile.Para[ctx->controlFile.CurrentNum][5];
	}
//
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][7]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	// @note M.KUNO 2022.12.02 original code
	//if( ctx->controlFile.Para[ctx->controlFile.CurrentNum][8] != 0.0 )	ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(0.3*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
	// @note M.KUNO 2022.12.02 edited code
	if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][8] != 0.0)	ctx->ao.raw[ctx->daCh.EP_Cell] = ctx->ao.raw[ctx->daCh.EP_Cell] + float(0.9 * ctx->ao.cal.a[ctx->daCh.EP_Cell] * (ctx->target.sr - ctx->phys.sr));
//
	if(ctx->phys.sz > ctx->target.sz+ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
	else if(ctx->phys.sz < ctx->target.sz-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
	else										ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
//	
	if(ctx->phys.szq > ctx->target.tzq + ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.TorsionDirection]=ctx->volt.CCW;
	else if(ctx->phys.szq < ctx->target.tzq - ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.TorsionDirection]=ctx->volt.CW;
	else											ctx->ao.raw[ctx->daCh.TorsionSpeed]=0.0f;
//
	if( fabs(ctx->phys.sz-ctx->controlFile.Para[ctx->controlFile.CurrentNum][3])<=ctx->err.StressMotor*2.0 && fabs(ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][4])<=ctx->err.StressMotor*2.0 && fabs(ctx->phys.szq-ctx->controlFile.Para[ctx->controlFile.CurrentNum][5])<=ctx->err.StressMotor*2.0 ){
		ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
		ctx->StepTime=0.0;
	}
}

void CDigitShowBasicDoc::MonotonicTorsionalLoading()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: sigma_zq
	//	2: gamma_zq
	//	3: Torsinal Speed
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.TorsionMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
//
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->phys.szq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.gzq1 < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
		else {
			ctx->StepTime=0.0;
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->phys.szq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.gzq1 > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
		else {
			ctx->StepTime=0.0;
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
		}
	}
}

void CDigitShowBasicDoc::MonotonicTorsionalLoadingCNS()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: sigma_zq
	//	2: gamma_zq
	//	3: Torsinal Speed
	//	4: Axial Speed
	//	5: Cell Pressure Rate
	//	6: sigma_z
	//	7: sigma_r
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	if(ctx->phys.sz > ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
	else if(ctx->phys.sz < ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
	else												ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
	ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][7];
	// @note M.KUNO 2022.12.15 original code
	//if( ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] != 0.0 )	ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(0.3*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
	// @note H.Hashimoto 2022.12.15 fixed code
	if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] != 0.0) {
		if (ctx->phys.sr < ctx->target.sr - ctx->err.StressAir) {
			ctx->ao.raw[ctx->daCh.EP_Cell] = ctx->ao.raw[ctx->daCh.EP_Cell] + float(ctx->ao.cal.a[ctx->daCh.EP_Cell] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] / 60.0 * ctx->timeSettings.Interval2 / 1000.0);
		}
		else if (ctx->phys.sr > ctx->target.sr + ctx->err.StressAir) {
			ctx->ao.raw[ctx->daCh.EP_Cell] = ctx->ao.raw[ctx->daCh.EP_Cell] - float(ctx->ao.cal.a[ctx->daCh.EP_Cell] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] / 60.0 * ctx->timeSettings.Interval2 / 1000.0);
		}
	}
//
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->phys.szq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.gzq1 < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
		else {
			ctx->StepTime=0.0;
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->phys.szq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.gzq1 > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])	ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
		else {
			ctx->StepTime=0.0;
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
		}
	}
}

void CDigitShowBasicDoc::CyclicTorsionalLoading()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: sigma_zq_lower
	//	2: sigma_zq_upper
	//	3: gamma_zq_lower
	//	4: gamma_zq_upper
	//	5: Number
	//	6: Torsinal Speed
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->NumCyclic==0){
			ctx->flags.Cyclic=FALSE;
			ctx->NumCyclic=1;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){
			if(ctx->flags.Cyclic==FALSE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.szq >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] || ctx->phys.gzq1 >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]) ctx->flags.Cyclic=TRUE;
			}
			if(ctx->flags.Cyclic==TRUE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.szq <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] || ctx->phys.gzq1 <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]){
					ctx->flags.Cyclic=FALSE;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->NumCyclic==0){
			ctx->flags.Cyclic=TRUE;
			ctx->NumCyclic=1;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){
			if(ctx->flags.Cyclic==FALSE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.szq <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] || ctx->phys.gzq1 <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) {
					ctx->flags.Cyclic=TRUE;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
			if(ctx->flags.Cyclic==TRUE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.szq >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] || ctx->phys.gzq1 <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]){
					ctx->flags.Cyclic=FALSE;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	} 
}

void CDigitShowBasicDoc::CyclicTorsionalLoadingCNS()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: sigma_zq_lower
	//	2: sigma_zq_upper
	//	3: gamma_zq_lower
	//	4: gamma_zq_upper
	//	5: Number
	//	6: Torsinal Speed
	//	7: Axial Speed
	//	8: Cell Pressure Rate
	//	9: sigma_z
	//	10: sigma_r
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][7]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	if(ctx->phys.sz > ctx->controlFile.Para[ctx->controlFile.CurrentNum][9]+ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
	else if(ctx->phys.sz < ctx->controlFile.Para[ctx->controlFile.CurrentNum][9]-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
	else												ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
	ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][10];
	if( ctx->controlFile.Para[ctx->controlFile.CurrentNum][8] != 0.0 )	ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(0.3*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
//	
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->NumCyclic==0){
			ctx->flags.Cyclic=FALSE;
			ctx->NumCyclic=1;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){
			if(ctx->flags.Cyclic==FALSE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.szq >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] || ctx->phys.gzq1 >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]) ctx->flags.Cyclic=TRUE;
			}
			if(ctx->flags.Cyclic==TRUE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.szq <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] || ctx->phys.gzq1 <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]){
					ctx->flags.Cyclic=FALSE;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->NumCyclic==0){
			ctx->flags.Cyclic=TRUE;
			ctx->NumCyclic=1;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){
			if(ctx->flags.Cyclic==FALSE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.szq <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] || ctx->phys.gzq1 <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) {
					ctx->flags.Cyclic=TRUE;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
			if(ctx->flags.Cyclic==TRUE){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.szq >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] || ctx->phys.gzq1 <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]){
					ctx->flags.Cyclic=FALSE;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	} 
}

void CDigitShowBasicDoc::SmallCyclicTorsionalLoading()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: Delta[gamma_zq]
	//	2: Number
	//	3: Torsinal Speed
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->NumCyclic==0){
			ctx->target.gzq = ctx->phys.gzq2;
			ctx->NumCyclic=1;
			ctx->NumSmallCyclic=0;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->NumSmallCyclic==0){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.gzq2 <= ctx->target.gzq - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) ctx->NumSmallCyclic=1;
			}
			if(ctx->NumSmallCyclic==1){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.gzq2 >= ctx->target.gzq + ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) ctx->NumSmallCyclic=2;
			}
			if(ctx->NumSmallCyclic==2){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.gzq2 <= ctx->target.gzq){;
					ctx->NumSmallCyclic=0;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->NumCyclic==0){
			ctx->target.gzq = ctx->phys.gzq2;
			ctx->NumCyclic=1;
			ctx->NumSmallCyclic=0;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->NumSmallCyclic==0){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.gzq2 >= ctx->target.gzq + ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])	ctx->NumSmallCyclic=1;
			}
			if(ctx->NumSmallCyclic==1){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.gzq2 <= ctx->target.gzq - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])	ctx->NumSmallCyclic=2;
			}
			if(ctx->NumSmallCyclic==2){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.gzq2 >= ctx->target.gzq) {
					ctx->NumSmallCyclic=0;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	} 
}


void CDigitShowBasicDoc::SmallCyclicTorsionalLoadingCNS()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: Delta[gamma_zq]
	//	2: Number
	//	3: Torsinal Speed
	//	4: Axial Speed
	//	5: Cell Pressure Rate
	//	6: sigma_z
	//	7: sigma_r
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	if(ctx->phys.sz > ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
	else if(ctx->phys.sz < ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
	else												ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
	ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][7];
	if( ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] != 0.0 )	ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(0.3*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
//
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->NumCyclic==0){
			ctx->target.gzq = ctx->phys.gzq2;
			ctx->NumCyclic=1;
			ctx->NumSmallCyclic=0;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->NumSmallCyclic==0){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.gzq2 <= ctx->target.gzq - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) ctx->NumSmallCyclic=1;
			}
			if(ctx->NumSmallCyclic==1){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.gzq2 >= ctx->target.gzq + ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) ctx->NumSmallCyclic=2;
			}
			if(ctx->NumSmallCyclic==2){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.gzq2 <= ctx->target.gzq){;
					ctx->NumSmallCyclic=0;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->NumCyclic==0){
			ctx->target.gzq = ctx->phys.gzq2;
			ctx->NumCyclic=1;
			ctx->NumSmallCyclic=0;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->NumSmallCyclic==0){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.gzq2 >= ctx->target.gzq + ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])	ctx->NumSmallCyclic=1;
			}
			if(ctx->NumSmallCyclic==1){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
				if(ctx->phys.gzq2 <= ctx->target.gzq - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])	ctx->NumSmallCyclic=2;
			}
			if(ctx->NumSmallCyclic==2){
				ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
				if(ctx->phys.gzq2 >= ctx->target.gzq) {
					ctx->NumSmallCyclic=0;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	} 
}

void CDigitShowBasicDoc::MonotonicAxialLoading()
{	DigitShowContext* ctx = GetContext();
	// 0: Loading:0/Unloading:1, 
	// 1: sigma_z (kPa), 
	// 2: epsilon_z,
	// 3: Axial Speed
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->phys.sz <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]) ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
		else {
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->phys.sz >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]) ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
		else {
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
		}
	}
}


void CDigitShowBasicDoc::CyclicAxialLoading()
{	DigitShowContext* ctx = GetContext();
	// 0: Loading:0/Unloading:1, 
	// 1: sigma_z_lower, 
	// 2: sigma_z_upper, 
	// 3: epsilon_z_lower,
	// 4: epsilon_z_upper,
	// 5: Number,
	// 6: Axial Speed
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->NumCyclic==0){
			ctx->flags.Cyclic=FALSE;
			ctx->NumCyclic=1;
		}
		if(ctx->NumCyclic!=0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){
			if(ctx->flags.Cyclic==FALSE){
				if(ctx->phys.sz <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->phys.ez <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4])	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
				else	ctx->flags.Cyclic=TRUE;
			}
			else {
				if(ctx->phys.sz >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
				else{
					ctx->flags.Cyclic=FALSE;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic>ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->NumCyclic==0){
			ctx->flags.Cyclic=TRUE;
			ctx->NumCyclic=1;
		}
		if(ctx->NumCyclic!=0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->flags.Cyclic==FALSE){
				if(ctx->phys.sz <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->phys.ez <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4])	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
				else{
					ctx->flags.Cyclic=TRUE;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
			else{
				if(ctx->phys.sz >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
				else	ctx->flags.Cyclic=FALSE;
			}
		}
		if(ctx->NumCyclic>ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	} 
}

void CDigitShowBasicDoc::SmallCyclicAxialLoading()
{	DigitShowContext* ctx = GetContext();
	//	0: Loading:0 / Unloading:1
	//	1: Delta[epsilon_z]
	//	2: Number
	//	3: Axial Speed
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->NumCyclic==0){
			ctx->target.ez = ctx->phys.ez;
			ctx->NumCyclic=1;
			ctx->NumSmallCyclic=0;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->NumSmallCyclic==0){
				ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Down;
				if(ctx->phys.ez >= ctx->target.ez + ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) ctx->NumSmallCyclic=1;
			}
			if(ctx->NumSmallCyclic==1){
				ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Up;
				if(ctx->phys.ez <= ctx->target.ez - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) ctx->NumSmallCyclic=2;
			}
			if(ctx->NumSmallCyclic==2){
				ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Down;
				if(ctx->phys.ez >= ctx->target.ez){;
					ctx->NumSmallCyclic=0;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->NumCyclic==0){
			ctx->target.ez = ctx->phys.ez;
			ctx->NumCyclic=1;
			ctx->NumSmallCyclic=0;
		}
		if(ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){
			if(ctx->NumSmallCyclic==0){
				ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Up;
				if(ctx->phys.ez <= ctx->target.ez - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])	ctx->NumSmallCyclic=1;
			}
			if(ctx->NumSmallCyclic==1){
				ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Down;
				if(ctx->phys.ez >= ctx->target.ez - ctx->controlFile.Para[ctx->controlFile.CurrentNum][1])	ctx->NumSmallCyclic=2;
			}
			if(ctx->NumSmallCyclic==2){
				ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Up;
				if(ctx->phys.ez <= ctx->target.ez) {
					ctx->NumSmallCyclic=0;
					ctx->NumCyclic=ctx->NumCyclic+1;
				}
			}
		}
		if(ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]){ 
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
			ctx->NumCyclic=0;
		}
	} 
}
void CDigitShowBasicDoc::Creep()
{	DigitShowContext* ctx = GetContext();
	// 0: sigma_z, 
	// 1: sigma_r,
	// 2: sigma_zq
	// 3: time (min)
	// 4: Torsinal Speed
	// 5: Axial Speed
	// 6: Cell Pressure Rate
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
//
	if(ctx->phys.sz > ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]+ctx->err.StressMotor)			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
	else if(ctx->phys.sz < ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
	else												ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
	ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][1];
	if( ctx->controlFile.Para[ctx->controlFile.CurrentNum][6] != 0.0 )	{
		if(ctx->phys.sr < ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]-ctx->err.StressAir) ctx->target.sr=ctx->phys.sr+ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]*ctx->timeSettings.Interval2/1000/60;
		if(ctx->phys.sr > ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]+ctx->err.StressAir) ctx->target.sr=ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]*ctx->timeSettings.Interval2/1000/60;
		if(fabs(ctx->phys.sr-ctx->controlFile.Para[ctx->controlFile.CurrentNum][1]) <= ctx->err.StressAir) ctx->target.sr=ctx->controlFile.Para[ctx->controlFile.CurrentNum][1];
		ctx->ao.raw[ctx->daCh.EP_Cell]=ctx->ao.raw[ctx->daCh.EP_Cell]+float(0.3*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
	}
	if(ctx->phys.szq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]+ctx->err.StressMotor)		ctx->ao.raw[ctx->daCh.TorsionDirection]=ctx->volt.CCW;
	else if(ctx->phys.szq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.TorsionDirection]=ctx->volt.CW;
	else												ctx->ao.raw[ctx->daCh.TorsionSpeed]=0.0f;
	if(ctx->StepTime >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]){
		ctx->StepTime=0.0;
		ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
	} 
}

void CDigitShowBasicDoc::MonotonicAxialLoadingConstP()
{	DigitShowContext* ctx = GetContext();
	// 0: Compression:0/Extension:1, 
	// 1: sigma_z (kPa), 
	// 2: epsilon_z,
	// 3: Axial Speed,
	// 4: Mean effective principal stress ctx->phys.p' (kPa), 
	// 5: sigma_zq (kPa),
	// 6: Torsional Speed,
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);
	if(ctx->phys.szq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]+ctx->err.StressMotor)		ctx->ao.raw[ctx->daCh.TorsionDirection]=ctx->volt.CCW;
	else if(ctx->phys.szq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.TorsionDirection]=ctx->volt.CW;
	else												ctx->ao.raw[ctx->daCh.TorsionSpeed]=0.0f;
	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->phys.sz <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]) {
			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
			ctx->target.sr= (3.0*ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]-ctx->phys.sz)/2.0 +ctx->err.StressAir;
			ctx->ao.raw[ctx->daCh.EP_Cell]= ctx->ao.raw[ctx->daCh.EP_Cell] + float(0.1*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
		}
		else {
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
		}
	}
	else if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==1.0){
		if(ctx->phys.sz >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]) {
			ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
			ctx->target.sr= (3.0*ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]-ctx->phys.sz)/2.0 -ctx->err.StressAir ;
			ctx->ao.raw[ctx->daCh.EP_Cell]= ctx->ao.raw[ctx->daCh.EP_Cell] + float(0.1*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
		}
		else {
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
			ctx->StepTime=0.0;
		}
	}
}

void CDigitShowBasicDoc::MonotonicTorsionalLoadingConstPA()
{	DigitShowContext* ctx = GetContext();
	//	0: Clockwise:0 / Countercloclwise:1
	//	1: sigma_zq
	//	2: gamma_zq
	//	3: Torsinal Speed
	//	4: Axial Speed
	//	5: Mean effective principal stress ctx->phys.p' (kPa),
	//	6: tan(2*alfa),
	ctx->StepTime=ctx->StepTime+ctx->CtrlStepTime/60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed]=float(ctx->ao.cal.a[ctx->daCh.AxisSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][4]+ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	ctx->ao.raw[ctx->daCh.TorsionMotor]=5.0f;
	ctx->ao.raw[ctx->daCh.TorsionSpeed]=float(ctx->ao.cal.a[ctx->daCh.TorsionSpeed]*ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]+ctx->ao.cal.b[ctx->daCh.TorsionSpeed]);

	if(ctx->controlFile.Para[ctx->controlFile.CurrentNum][0]==0.0){
		if(ctx->phys.szq < ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.gzq1 < ctx->controlFile.Para[ctx->controlFile.CurrentNum][2])	{
			ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CW;
			ctx->target.sr= (3.0*ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] - 2.0*ctx->phys.szq/ctx->controlFile.Para[ctx->controlFile.CurrentNum][6])/3.0 +ctx->err.StressAir;
			ctx->ao.raw[ctx->daCh.EP_Cell]= ctx->ao.raw[ctx->daCh.EP_Cell] + float(0.1*ctx->ao.cal.a[ctx->daCh.EP_Cell]*(ctx->target.sr-ctx->phys.sr));
			ctx->target.sz= (3.0*ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] + 4.0*ctx->phys.szq/ctx->controlFile.Para[ctx->controlFile.CurrentNum][6])/3.0 +ctx->err.StressMotor;
			if(ctx->phys.sz > ctx->target.sz+ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
			else if(ctx->phys.sz < ctx->target.sz-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
			else	ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
		}
		else {
			ctx->StepTime = 0.0;
			ctx->controlFile.CurrentNum = ctx->controlFile.CurrentNum + 1;
		}
	}
	else if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] == 1.0) {
		if (ctx->phys.szq > ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.gzq1 > ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]) {
			ctx->ao.raw[ctx->daCh.TorsionDirection] = ctx->volt.CCW;
			ctx->target.sr = (3.0 * ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] - 2.0 * ctx->phys.szq / ctx->controlFile.Para[ctx->controlFile.CurrentNum][6]) / 3.0 - ctx->err.StressAir;
			ctx->ao.raw[ctx->daCh.EP_Cell] = ctx->ao.raw[ctx->daCh.EP_Cell] + float(0.1 * ctx->ao.cal.a[ctx->daCh.EP_Cell] * (ctx->target.sr - ctx->phys.sr));
			ctx->target.sz= (3.0*ctx->controlFile.Para[ctx->controlFile.CurrentNum][5] + 4.0*ctx->phys.szq/ctx->controlFile.Para[ctx->controlFile.CurrentNum][6])/3.0 -ctx->err.StressMotor;
			if(ctx->phys.sz > ctx->target.sz+ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Up;
			else if(ctx->phys.sz < ctx->target.sz-ctx->err.StressMotor)	ctx->ao.raw[ctx->daCh.AxisDirection]=ctx->volt.Down;
			else	ctx->ao.raw[ctx->daCh.AxisSpeed]=0.0f;
		}
		else {
			ctx->StepTime=0.0;
			ctx->controlFile.CurrentNum=ctx->controlFile.CurrentNum+1;
		}
	}
}

// 2021.06.07 Edited by M.Kuno
// customize for Sanjei	
void CDigitShowBasicDoc::CyclicAxialLoading_OR()
{	DigitShowContext* ctx = GetContext();
	// 0: Loading:0/Unloading:1, 
	// 1: sigma_z_lower, 
	// 2: sigma_z_upper, 
	// 3: epsilon_z_lower,
	// 4: epsilon_z_upper,
	// 5: Number,
	// 6: Axial Speed
	ctx->StepTime = ctx->StepTime + ctx->CtrlStepTime / 60.0;
	ctx->ao.raw[ctx->daCh.AxisMotor] = 5.0f;
	ctx->ao.raw[ctx->daCh.AxisSpeed] = float(ctx->ao.cal.a[ctx->daCh.AxisSpeed] * ctx->controlFile.Para[ctx->controlFile.CurrentNum][6] + ctx->ao.cal.b[ctx->daCh.AxisSpeed]);
	if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] == 0.0) {
		if (ctx->NumCyclic == 0) {
			ctx->flags.Cyclic = FALSE;
			ctx->NumCyclic = 1;
		}
		if (ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) {
			if (ctx->flags.Cyclic == FALSE) {
				if (ctx->phys.sz <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] && ctx->phys.ez <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4])	ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Down;
				else	ctx->flags.Cyclic = TRUE;
			}
			else {
				if (ctx->phys.sz >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] || ctx->phys.ez >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Up;
				else {
					ctx->flags.Cyclic = FALSE;
					ctx->NumCyclic = ctx->NumCyclic + 1;
				}
			}
		}
		if (ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) {
			ctx->controlFile.CurrentNum = ctx->controlFile.CurrentNum + 1;
			ctx->StepTime = 0.0;
			ctx->NumCyclic = 0;
		}
	}
	else if (ctx->controlFile.Para[ctx->controlFile.CurrentNum][0] == 1.0) {
		if (ctx->NumCyclic == 0) {
			ctx->flags.Cyclic = TRUE;
			ctx->NumCyclic = 1;
		}
		if (ctx->NumCyclic != 0 && ctx->NumCyclic <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2]) {
			if (ctx->flags.Cyclic == FALSE) {
				if (ctx->phys.sz <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][2] || ctx->phys.ez <= ctx->controlFile.Para[ctx->controlFile.CurrentNum][4])	ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Down;
				else {
					ctx->flags.Cyclic = TRUE;
					ctx->NumCyclic = ctx->NumCyclic + 1;
				}
			}
			else {
				if (ctx->phys.sz >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][1] && ctx->phys.ez >= ctx->controlFile.Para[ctx->controlFile.CurrentNum][3]) ctx->ao.raw[ctx->daCh.AxisDirection] = ctx->volt.Up;
				else	ctx->flags.Cyclic = FALSE;
			}
		}
		if (ctx->NumCyclic > ctx->controlFile.Para[ctx->controlFile.CurrentNum][5]) {
			ctx->controlFile.CurrentNum = ctx->controlFile.CurrentNum + 1;
			ctx->StepTime = 0.0;
			ctx->NumCyclic = 0;
		}
	}
}


