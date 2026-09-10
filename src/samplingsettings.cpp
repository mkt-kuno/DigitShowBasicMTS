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
// SamplingSettings.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "DigitShowBasic.h"
#include "DigitShowBasicDoc.h"
#include "SamplingSettings.h"
#include "DigitShowContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSamplingSettings::CSamplingSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CSamplingSettings::IDD, pParent)
{
	m_TimeInterval1 = 0;
	m_TimeInterval2 = 0;
	m_TimeInterval3 = 0;
	m_TotalSamplingTimes = 0;
	m_AllocatedMemory = _T("");
	m_AvSmplNum = 0;
	m_Channels = 0;
	m_EventSamplingTimes = 0;
	m_MemoryType = _T("");
	m_SamplingClock = 0.0f;
	m_SavingTime = 0;
}

void CSamplingSettings::DoDataExchange(CDataExchange* pDX)
{   DigitShowContext* ctx = GetContext();
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TimeInterval1, m_TimeInterval1);
	DDX_Text(pDX, IDC_EDIT_TimeInterval2, m_TimeInterval2);
	DDX_Text(pDX, IDC_EDIT_TimeInterval3, m_TimeInterval3);
	DDX_Text(pDX, IDC_EDIT_TotalSamplingTimes, m_TotalSamplingTimes);
	DDX_Text(pDX, IDC_EDIT_AllocatedMemory, m_AllocatedMemory);
	DDX_Text(pDX, IDC_EDIT_AvSmplNum, m_AvSmplNum);
	DDX_Text(pDX, IDC_EDIT_Channels, m_Channels);
	DDX_Text(pDX, IDC_EDIT_EventSamplingTimes, m_EventSamplingTimes);
	DDX_Text(pDX, IDC_EDIT_MemoryType, m_MemoryType);
	DDX_Text(pDX, IDC_EDIT_SamplingClock, m_SamplingClock);
	DDX_Text(pDX, IDC_EDIT_SavingTime, m_SavingTime);
}

BEGIN_MESSAGE_MAP(CSamplingSettings, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_Check, OnBUTTONCheck)
END_MESSAGE_MAP()

BOOL CSamplingSettings::OnInitDialog() 
{   DigitShowContext* ctx = GetContext();
	CDialog::OnInitDialog();
	m_TimeInterval1 = ctx->timeSettings.Interval1;
	m_TimeInterval2 = ctx->timeSettings.Interval2;
	m_TimeInterval3 = ctx->timeSettings.Interval3;
	OnBUTTONCheck();
	return TRUE;
}

void CSamplingSettings::OnBUTTONCheck() 
{
	UpdateData(TRUE);
	m_MemoryType = _T("Modbus polling");
	m_Channels = AI_MAX_CHANNELS;
	m_EventSamplingTimes = AO_MAX_CHANNELS;
	m_AvSmplNum = 1;
	m_TotalSamplingTimes = 0;
	m_AllocatedMemory = _T("N/A");
	m_SamplingClock = (float)m_TimeInterval1;
	m_SavingTime = (int)(m_TimeInterval3 / 1000);
	UpdateData(FALSE);
}

void CSamplingSettings::OnOK() 
{   DigitShowContext* ctx = GetContext();
	UpdateData(TRUE);
	if (m_TimeInterval1 > 0) ctx->timeSettings.Interval1 = (unsigned int)m_TimeInterval1;
	if (m_TimeInterval2 > 0) ctx->timeSettings.Interval2 = (unsigned int)m_TimeInterval2;
	if (m_TimeInterval3 > 0) ctx->timeSettings.Interval3 = (unsigned int)m_TimeInterval3;
	CDialog::OnOK();
}