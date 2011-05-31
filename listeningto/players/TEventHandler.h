/*
Generic event handler template class for non-ATL clients.

Article: Understanding COM Event Handling
         http://www.codeproject.com/KB/COM/TEventHandler.aspx
Autor:   Lim Bio Liong
         http://www.codeproject.com/Members/Lim-Bio-Liong 

==========================================================================
ListeningTo plugin for Miranda IM

Copyright	(C) 2005-2011 Ricardo Pescuma Domenecci
			(C) 2010-2011 Merlin_de

PRE-CONDITION to use this code under the GNU General Public License:
 1. you do not build another Miranda IM plugin with the code without written permission
    of the autor (peace for the project).
 2. you do not publish copies of the code in other Miranda IM-related code repositories.
    This project is already hosted in a SVN and you are welcome to become a contributing member.
 3. you do not create listeningTo-derivatives based on this code for the Miranda IM project.
    (feel free to do this for another project e.g. foobar)
 4. you do not distribute any kind of self-compiled binary of this plugin (we want continuity
    for the plugin users, who should know that they use the original) you can compile this plugin
    for your own needs, friends, but not for a whole branch of people (e.g. miranda plugin pack).
 5. This isn't free beer. If your jurisdiction (country) does not accept
    GNU General Public License, as a whole, you have no rights to the software
    until you sign a private contract with its author. !!!
 6. you always put these notes and copyright notice at the beginning of your code.
==========================================================================

in case you accept the pre-condition,
this is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef TEVENT_HANDLER_H
#define TEVENT_HANDLER_H

#include <windows.h>

namespace TEventHandlerNamespace
{
// Generic event handler template class (especially useful (but not limited to) for non-ATL clients).
template <class event_handler_class, typename device_interface, typename device_event_interface>
class TEventHandler : IDispatch
{
  friend class class_event_handler;

  typedef HRESULT (event_handler_class::*parent_on_invoke)
  (
//	TEventHandler<event_handler_class, device_interface, device_event_interface>* pthis,
	void* pthis,
	DISPID dispidMember, 
	REFIID riid,
	LCID lcid, 
	WORD wFlags, 
	DISPPARAMS* pdispparams, 
	VARIANT* pvarResult,
	EXCEPINFO* pexcepinfo, 
	UINT* puArgErr
  );

public :
  TEventHandler
  (
	event_handler_class& parent,
	device_interface* pdevice_interface,  // Non-ref counted.
	parent_on_invoke parent_on_invoke_function
  ) :
	m_cRef(1),
	m_parent(parent),
	m_parent_on_invoke(parent_on_invoke_function),
	m_pIConnectionPoint(0),
	m_dwEventCookie(0),
	m_vbServerState(VARIANT_TRUE)

  {
	SetupConnectionPoint(pdevice_interface);
  }
	   
  ~TEventHandler()
  {
	// Call ShutdownConnectionPoint() here JUST IN CASE connection points are still 
	// alive at this time. They should have been disconnected earlier.
	ShutdownConnectionPoint();
  }

  STDMETHOD_(ULONG, AddRef)()
  {
	InterlockedIncrement(&m_cRef);
	return m_cRef;  
  }

  STDMETHOD_(ULONG, Release)()
  {
	if(m_cRef)
		InterlockedDecrement(&m_cRef);
	if(m_cRef == 1) {
		DISPPARAMS dispparams;
		ZeroMemory(&dispparams, sizeof dispparams);
		m_vbServerState = VARIANT_FALSE;
		(m_parent.*m_parent_on_invoke)(this, (DISPID)0x9999, IID_NULL, 0, 0, &dispparams, NULL, NULL, NULL);
	}
	else if(m_cRef == 0) {
		delete this;
		return 0;
	}

	return m_cRef;
  }
	   
  STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject)
  {
	if (riid == IID_IUnknown)
	{
		*ppvObject = (IUnknown*)this;
		AddRef();
		return S_OK;
	}
	
	if ((riid == IID_IDispatch) || (riid == __uuidof(device_event_interface)))
	{
		*ppvObject = (IDispatch*)this;
		AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
  }

  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
  {
	return E_NOTIMPL;
  }

  STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
  {
	return E_NOTIMPL;
  }

  STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		   LCID lcid, DISPID* rgdispid)
  {
	return E_NOTIMPL;
  }

  STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		   LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		   EXCEPINFO* pexcepinfo, UINT* puArgErr)
  {
	return (m_parent.*m_parent_on_invoke)(this, dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
  }


protected :
  LONG						m_cRef;

  // Pertaining to the owner of this object.
  event_handler_class&		m_parent;  // Non-reference counted. This is to prevent circular references.

  // Pertaining to connection points.
  IConnectionPoint*			m_pIConnectionPoint;  // Ref counted of course.
  DWORD						m_dwEventCookie;
  parent_on_invoke			m_parent_on_invoke;

  void SetupConnectionPoint(device_interface* pdevice_interface)
  {
    IConnectionPointContainer*	pIConnectionPointContainerTemp = NULL;
    IUnknown*					pIUnknown = NULL;

    // QI this object itself for its IUnknown pointer which will be used 
    // later to connect to the Connection Point of the device_interface object.
    this -> QueryInterface(IID_IUnknown, (void**)&pIUnknown);

    if (pIUnknown)
    {
      // QI the pdevice_interface for its connection point.
      pdevice_interface -> QueryInterface (IID_IConnectionPointContainer, (void**)&pIConnectionPointContainerTemp);

      if (pIConnectionPointContainerTemp)
      {
	    pIConnectionPointContainerTemp -> FindConnectionPoint(__uuidof(device_event_interface), &m_pIConnectionPoint);
	    pIConnectionPointContainerTemp -> Release();
	    pIConnectionPointContainerTemp = NULL;
      }

      if (m_pIConnectionPoint)
      {
	    m_pIConnectionPoint -> Advise(pIUnknown, &m_dwEventCookie);
      }

      pIUnknown -> Release();
	  pIUnknown = NULL;
    }
  }

public :
  VARIANT_BOOL m_vbServerState;

  void ShutdownConnectionPoint()
  {
	if (m_pIConnectionPoint && m_cRef>1) {
		HRESULT hr = NULL;
		//client shutdown ConnectionPoint
		if(	m_vbServerState == VARIANT_TRUE && 
			SUCCEEDED(m_pIConnectionPoint -> Unadvise(m_dwEventCookie)) )
		{
			m_dwEventCookie = 0;
			if(!m_pIConnectionPoint->Release())
				m_pIConnectionPoint = NULL;
		}
		//else server destroy ConnectionPoint (all obj invalid)
		//m_pIConnectionPoint is only a pointer and c++ destructor destroy it
		//if we try to Unadvise or Release it at this stage we alwas get exeption
		return;
	}
  }
};

};

#endif

