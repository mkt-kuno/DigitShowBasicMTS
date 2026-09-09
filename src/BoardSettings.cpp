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
// BoardSettings.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "DigitShowBasic.h"
#include "BoardSettings.h"
#include "DigitShowContext.h"
#include "ModbusRTU.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBoardSettings::CBoardSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CBoardSettings::IDD, pParent)
{
	m_AdMaxChannel1 = _T("");
	m_AdMaxChannel2 = _T("");
	m_AdMethod1 = _T("");
	m_AdMethod2 = _T("");
	m_AdRange1 = _T("");
	m_AdRange2 = _T("");
	m_AdResolution1 = _T("");
	m_AdResolution2 = _T("");
	m_DaMaxChannel = _T("");
	m_DaRange = _T("");
	m_DaResolution = _T("");
}

void CBoardSettings::DoDataExchange(CDataExchange* pDX)
{   DigitShowContext* ctx = GetContext();
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_AdMaxChannel1, m_AdMaxChannel1);
	DDX_Text(pDX, IDC_EDIT_AdMaxChannel2, m_AdMaxChannel2);
	DDX_Text(pDX, IDC_EDIT_AdMethod1, m_AdMethod1);
	DDX_Text(pDX, IDC_EDIT_AdMethod2, m_AdMethod2);
	DDX_Text(pDX, IDC_EDIT_AdRange1, m_AdRange1);
	DDX_Text(pDX, IDC_EDIT_AdRange2, m_AdRange2);
	DDX_Text(pDX, IDC_EDIT_AdResolution1, m_AdResolution1);
	DDX_Text(pDX, IDC_EDIT_AdResolution2, m_AdResolution2);
	DDX_Text(pDX, IDC_EDIT_DaMaxChannel, m_DaMaxChannel);
	DDX_Text(pDX, IDC_EDIT_DaRange, m_DaRange);
	DDX_Text(pDX, IDC_EDIT_DaResolution, m_DaResolution);
}

BEGIN_MESSAGE_MAP(CBoardSettings, CDialog)
END_MESSAGE_MAP()

BOOL CBoardSettings::OnInitDialog() 
{   DigitShowContext* ctx = GetContext();
	CDialog::OnInitDialog();
	ModbusRTU* modbus = GetModbusInstance();
	m_AdMethod1 = _T("HX711");
	m_AdResolution1 = _T("16-bit signed");
	m_AdRange1 = (ctx->flags.SetBoard && modbus->IsOpen()) ? _T("Connected") : _T("Not connected");
	m_AdMaxChannel1 = _T("8 (ch0-7)");
	m_AdMethod2 = _T("ADS1115");
	m_AdResolution2 = _T("16-bit signed");
	m_AdRange2 = _T("Modbus RTU 38400bps");
	m_AdMaxChannel2 = _T("8 (ch8-15)");
	m_DaResolution = _T("GP8403");
	m_DaRange = _T("0-10000 mV");
	m_DaMaxChannel = _T("8 (ch0-7)");
	UpdateData(FALSE);
	return TRUE;
}