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
#if !defined(AFX_SAMPLINGSETTINGS_H__7C23E5A9_7E1B_4821_857B_F8600C2F6EA6__INCLUDED_)
#define AFX_SAMPLINGSETTINGS_H__7C23E5A9_7E1B_4821_857B_F8600C2F6EA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

class CSamplingSettings : public CDialog
{
public:
	CSamplingSettings(CWnd* pParent = NULL);
	enum { IDD = IDD_SamplingSettings };
	long	m_TimeInterval1;
	long	m_TimeInterval2;
	long	m_TimeInterval3;
	long	m_TotalSamplingTimes;
	CString	m_AllocatedMemory;
	int		m_AvSmplNum;
	int		m_Channels;
	int		m_EventSamplingTimes;
	CString	m_MemoryType;
	float	m_SamplingClock;
	int		m_SavingTime;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBUTTONCheck();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};

#endif