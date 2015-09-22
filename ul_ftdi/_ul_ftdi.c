/*
    Copyright (c) 2010 by god6or@gmail.com

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "_ul_ftdi.h"


/* Do not include the EEPROM functions for now. */
#define WITH_EEPROM_FUNCTIONS 0
// #define WITH_EEPROM_FUNCTIONS 1



// Get number of connected FTDI devices. Also rebuilds device info list.
static int _ftdi_ndevs(lua_State *L) {
    DWORD ndevs;
    FT_STATUS result = FT_CreateDeviceInfoList( &ndevs );
    if (result != FT_OK) ndevs = 0;
    lua_pushinteger(L, ndevs);
    return 1;
}

// Get information about connected FTDI devices.
static int _ftdi_devs(lua_State *L) {
    DWORD ndevs;
    FT_DEVICE_LIST_INFO_NODE* info;
    lua_newtable(L); // create table devices info tables

    FT_CreateDeviceInfoList( &ndevs );
    if (ndevs) {
        info = (FT_DEVICE_LIST_INFO_NODE *) malloc(ndevs * sizeof(FT_DEVICE_LIST_INFO_NODE));
        FT_STATUS result = FT_GetDeviceInfoList( info, &ndevs );
        if (result == FT_OK) {
            for (int i=0; i<ndevs; i++) {
                lua_pushinteger(L, i+1); // table index at +2
                lua_newtable(L); // create device info table at +1
                lua_pushinteger(L, info[i].Flags);
                lua_setfield(L, -2, "flags");
                lua_pushinteger(L, info[i].Type);
                lua_setfield(L, -2, "t");
                lua_pushinteger(L, info[i].ID);
                lua_setfield(L, -2, "id");
                lua_pushinteger(L, info[i].LocId);
                lua_setfield(L, -2, "lid");
                lua_pushinteger(L, (DWORD)info[i].ftHandle);
                lua_setfield(L, -2, "handle");
                lua_pushlstring(L, &(*info[i].SerialNumber), 16);
                lua_setfield(L, -2, "serial");
                lua_pushlstring(L, &(*info[i].Description), 64);
                lua_setfield(L, -2, "descr");
                lua_settable(L, -3); // append info table to main table, pop index and info
            }
        }
        free(info);
    }
    
    return 1;
}

// Open FTDI device. Returns opened handle or nil if error.
static int _ftdi_open(lua_State *L) {
    FT_HANDLE handle;
    FT_STATUS result = FT_OTHER_ERROR;
    if (lua_isnumber(L,1)) { // 1st is number
        int idev = lua_tointeger(L, 1);
        result = FT_Open( idev, &handle );
    }
    if (result == FT_OK) lua_pushinteger(L, (DWORD)handle);
    else lua_pushnil(L);
    return 1;
}

// Close FTDI device by its handle. Returns true if success, false if error.
static int _ftdi_close(lua_State *L) {
    int closed = 0;
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_STATUS result = FT_Close( handle );
        if (result == FT_OK) closed = 1;
    }
    lua_pushboolean(L, closed);
    return 1;
}

// Get number of bytes in receive queue. Returns nil if error.
static int _ftdi_availRX(lua_State *L) {
    int error = 1;
    DWORD qrx, qtx, evstat;
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_STATUS result = FT_GetStatus( handle, &qrx, &qtx, &evstat );
        if (result == FT_OK) error = 0;
    }
    if (error) lua_pushnil(L);
    else lua_pushinteger(L, qrx);
    return 1;
}
// Get number of bytes in transmit queue. Returns nil if error.
static int _ftdi_availTX(lua_State *L) {
    int error = 1;
    DWORD qrx, qtx, evstat;
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_STATUS result = FT_GetStatus( handle, &qrx, &qtx, &evstat );
        if (result == FT_OK) error = 0;
    }
    if (error) lua_pushnil(L);
    else lua_pushinteger(L, qtx);
    return 1;
}

// Flush receive queue. Returns nil if error.
static int _ftdi_flushRX(lua_State *L) {
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_Purge( handle, FT_PURGE_RX );
    }
    return 0;
}
// Flush transmit queue. Returns nil if error.
static int _ftdi_flushTX(lua_State *L) {
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_Purge( handle, FT_PURGE_TX );
    }
    return 0;
}


// Get latency timer.
static int _ftdi_latency(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    UCHAR latency;
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        result = FT_GetLatencyTimer( handle, &latency );
    }
    if (result != FT_OK) lua_pushnil(L);
    else lua_pushinteger(L, latency);
    return 1;
}
// Set latency timer.
static int _ftdi_setLatency(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    if ((lua_isnumber(L,1))&&(lua_isnumber(L,2))) {
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        int latency = lua_tointeger(L, 2);
        if ((latency >= 2)&&(latency <= 255)) result = FT_SetLatencyTimer(handle, latency);
    }
    lua_pushboolean(L, result == FT_OK);
    return 1;
}

// Get bit mode.
static int _ftdi_bm(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    UCHAR bm;
    if (lua_isnumber(L,1)) { // 1st is number
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        result = FT_GetBitMode( handle, &bm );
    }
    if (result != FT_OK) lua_pushnil(L);
    else lua_pushinteger(L, bm);
    return 1;
}
// Set bit mode.
static int _ftdi_setBm(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    if ((lua_isnumber(L,1))&&(lua_isnumber(L,2)&&(lua_isnumber(L,3)))) {
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        int bm = lua_tointeger(L, 2);
        int mask = lua_tointeger(L, 3);
        result = FT_SetBitMode(handle, mask, bm);
    }
    lua_pushboolean(L, result == FT_OK);
    return 1;
}


// Set baud rate.
static int _ftdi_setBaud(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    if ((lua_isnumber(L,1))&&(lua_isnumber(L,2))) {
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        int baud = lua_tointeger(L, 2);
        result = FT_SetBaudRate(handle, baud);
    }
    lua_pushboolean(L, result == FT_OK);
    return 1;
}
// Set baud rate divisor.
static int _ftdi_setDiv(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    if ((lua_isnumber(L,1))&&(lua_isnumber(L,2))) {
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        int div = lua_tointeger(L, 2);
        result = FT_SetDivisor(handle, div);
    }
    lua_pushboolean(L, result == FT_OK);
    return 1;
}


// Write data.
static int _ftdi_write(lua_State *L) {
    FT_STATUS result = FT_OTHER_ERROR;
    if ((lua_isnumber(L,1))&&(lua_isstring(L,2))) { // 1st is handle, 2nd is data string
        size_t ndata, wdata;
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        const char *data = lua_tolstring(L, 2, &ndata);
        if (ndata > 0) result = FT_Write( handle, (LPVOID)data, ndata, (DWORD*)&wdata );
    }
    lua_pushboolean(L, result == FT_OK);
    return 1;
}
// Read data.
static int _ftdi_read(lua_State *L) {
    DWORD ndata=0, rdata;
    char *data = NULL;
    if (lua_isnumber(L,1)) { // 1st is handle
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_STATUS result = FT_GetQueueStatus(handle, &ndata);
        if ((result==FT_OK)&&(ndata > 0)) {
            data = malloc(ndata);
            result = FT_Read(handle, data, ndata, &rdata);
            if (result == FT_OK) ndata = rdata;
            else ndata = 0;
        }
    }
    lua_pushlstring(L, data, ndata);
    if (data != NULL) free(data);
    return 1;
}


#if WITH_EEPROM_FUNCTIONS!=0
///////////////////////////////////////////////////////////////////////////////
// Read eeprom.
//   Input: opened device handle.
//   Returns: table with FT_PROGRAM_DATA fields or nil.
static int _ftdi_read_eep(lua_State *L) {
    if (lua_isnumber(L,1)) { // 1st is handle
        
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_PROGRAM_DATA ft_data;
        char ManufacturerBuf[32];
        char ManufacturerIdBuf[16];
        char DescriptionBuf[64];
        char SerialNumberBuf[16];

        ft_data.Signature1     = 0x00000000;
        ft_data.Signature2     = 0xffffffff;
        ft_data.Version        = 0x00000004; // EEPROM structure with FT4232H extensions
        ft_data.Manufacturer   = ManufacturerBuf;
        ft_data.ManufacturerId = ManufacturerIdBuf;
        ft_data.Description    = DescriptionBuf;
        ft_data.SerialNumber   = SerialNumberBuf;
        
        FT_STATUS status = FT_EE_Read( handle, &ft_data );
        if (status == FT_OK) {
            // prepare parsed table
            lua_newtable(L);
            lua_pushinteger(L, ft_data.VendorId);
            lua_setfield(L, -2, "VendorId");
            lua_pushinteger(L, ft_data.ProductId);
            lua_setfield(L, -2, "ProductId");

            lua_pushstring(L, ft_data.Manufacturer);
            lua_setfield(L, -2, "Manufacturer");
            lua_pushstring(L, ft_data.ManufacturerId);
            lua_setfield(L, -2, "ManufacturerId");
            lua_pushstring(L, ft_data.Description);
            lua_setfield(L, -2, "Description");
            lua_pushstring(L, ft_data.SerialNumber);
            lua_setfield(L, -2, "SerialNumber");

            lua_pushinteger(L, ft_data.MaxPower);
            lua_setfield(L, -2, "MaxPower");
            lua_pushinteger(L, ft_data.PnP);
            lua_setfield(L, -2, "PnP");
            lua_pushinteger(L, ft_data.SelfPowered);
            lua_setfield(L, -2, "SelfPowered");
            lua_pushinteger(L, ft_data.RemoteWakeup);
            lua_setfield(L, -2, "RemoteWakeup");

            // Rev4 extensions
            lua_pushinteger(L, ft_data.Rev4);
            lua_setfield(L, -2, "Rev4");
            lua_pushinteger(L, ft_data.IsoIn);
            lua_setfield(L, -2, "IsoIn");
            lua_pushinteger(L, ft_data.IsoOut);
            lua_setfield(L, -2, "IsoOut");
            lua_pushinteger(L, ft_data.PullDownEnable);
            lua_setfield(L, -2, "PullDownEnable");
            lua_pushinteger(L, ft_data.SerNumEnable);
            lua_setfield(L, -2, "SerNumEnable");
            lua_pushinteger(L, ft_data.USBVersionEnable);
            lua_setfield(L, -2, "USBVersionEnable");
            lua_pushinteger(L, ft_data.USBVersion);
            lua_setfield(L, -2, "USBVersion");

            // Rev5 (FT2232) extensions
            lua_pushinteger(L, ft_data.Rev5);
            lua_setfield(L, -2, "Rev5");
            lua_pushinteger(L, ft_data.IsoInA);
            lua_setfield(L, -2, "IsoInA");
            lua_pushinteger(L, ft_data.IsoInB);
            lua_setfield(L, -2, "IsoInB");
            lua_pushinteger(L, ft_data.IsoOutA);
            lua_setfield(L, -2, "IsoOutA");
            lua_pushinteger(L, ft_data.IsoOutB);
            lua_setfield(L, -2, "IsoOutB");
            lua_pushinteger(L, ft_data.PullDownEnable5);
            lua_setfield(L, -2, "PullDownEnable5");
            lua_pushinteger(L, ft_data.SerNumEnable5);
            lua_setfield(L, -2, "SerNumEnable5");
            lua_pushinteger(L, ft_data.USBVersionEnable5);
            lua_setfield(L, -2, "USBVersionEnable5");
            lua_pushinteger(L, ft_data.USBVersion5);
            lua_setfield(L, -2, "USBVersion5");
            lua_pushinteger(L, ft_data.AIsHighCurrent);
            lua_setfield(L, -2, "AIsHighCurrent");
            lua_pushinteger(L, ft_data.BIsHighCurrent);
            lua_setfield(L, -2, "BIsHighCurrent");
            lua_pushinteger(L, ft_data.IFAIsFifo);
            lua_setfield(L, -2, "IFAIsFifo");
            lua_pushinteger(L, ft_data.IFAIsFifoTar);
            lua_setfield(L, -2, "IFAIsFifoTar");
            lua_pushinteger(L, ft_data.IFAIsFastSer);
            lua_setfield(L, -2, "IFAIsFastSer");
            lua_pushinteger(L, ft_data.AIsVCP);
            lua_setfield(L, -2, "AIsVCP");
            lua_pushinteger(L, ft_data.IFBIsFifo);
            lua_setfield(L, -2, "IFBIsFifo");
            lua_pushinteger(L, ft_data.IFBIsFifoTar);
            lua_setfield(L, -2, "IFBIsFifoTar");
            lua_pushinteger(L, ft_data.IFBIsFastSer);
            lua_setfield(L, -2, "IFBIsFastSer");
            lua_pushinteger(L, ft_data.BIsVCP);
            lua_setfield(L, -2, "BIsVCP");

            // FT232R extensions
            lua_pushinteger(L, ft_data.UseExtOsc);
            lua_setfield(L, -2, "UseExtOsc");
            lua_pushinteger(L, ft_data.HighDriveIOs);
            lua_setfield(L, -2, "HighDriveIOs");
            lua_pushinteger(L, ft_data.EndpointSize);
            lua_setfield(L, -2, "EndpointSize");
            lua_pushinteger(L, ft_data.PullDownEnableR);
            lua_setfield(L, -2, "PullDownEnableR");
            lua_pushinteger(L, ft_data.SerNumEnableR);
            lua_setfield(L, -2, "SerNumEnableR");
            lua_pushinteger(L, ft_data.InvertTXD);
            lua_setfield(L, -2, "InvertTXD");
            lua_pushinteger(L, ft_data.InvertRXD);
            lua_setfield(L, -2, "InvertRXD");
            lua_pushinteger(L, ft_data.InvertRTS);
            lua_setfield(L, -2, "InvertRTS");
            lua_pushinteger(L, ft_data.InvertCTS);
            lua_setfield(L, -2, "InvertCTS");
            lua_pushinteger(L, ft_data.InvertDTR);
            lua_setfield(L, -2, "InvertDTR");
            lua_pushinteger(L, ft_data.InvertDSR);
            lua_setfield(L, -2, "InvertDSR");
            lua_pushinteger(L, ft_data.InvertDCD);
            lua_setfield(L, -2, "InvertDCD");
            lua_pushinteger(L, ft_data.InvertRI);
            lua_setfield(L, -2, "InvertRI");
            lua_pushinteger(L, ft_data.Cbus0);
            lua_setfield(L, -2, "Cbus0");
            lua_pushinteger(L, ft_data.Cbus1);
            lua_setfield(L, -2, "Cbus1");
            lua_pushinteger(L, ft_data.Cbus2);
            lua_setfield(L, -2, "Cbus2");
            lua_pushinteger(L, ft_data.Cbus3);
            lua_setfield(L, -2, "Cbus3");
            lua_pushinteger(L, ft_data.Cbus4);
            lua_setfield(L, -2, "Cbus4");

            lua_pushinteger(L, ft_data.RIsD2XX);
            lua_setfield(L, -2, "RIsD2XX");

            // Rev 7 (FT2232H) Extensions
            lua_pushinteger(L, ft_data.PullDownEnable7);
            lua_setfield(L, -2, "PullDownEnable7");
            lua_pushinteger(L, ft_data.SerNumEnable7);
            lua_setfield(L, -2, "SerNumEnable7");
            lua_pushinteger(L, ft_data.ALSlowSlew);
            lua_setfield(L, -2, "ALSlowSlew");
            lua_pushinteger(L, ft_data.ALSchmittInput);
            lua_setfield(L, -2, "ALSchmittInput");
            lua_pushinteger(L, ft_data.ALDriveCurrent);
            lua_setfield(L, -2, "ALDriveCurrent");
            lua_pushinteger(L, ft_data.AHSlowSlew);
            lua_setfield(L, -2, "AHSlowSlew");
            lua_pushinteger(L, ft_data.AHSchmittInput);
            lua_setfield(L, -2, "AHSchmittInput");
            lua_pushinteger(L, ft_data.AHDriveCurrent);
            lua_setfield(L, -2, "AHDriveCurrent");
            lua_pushinteger(L, ft_data.BLSlowSlew);
            lua_setfield(L, -2, "BLSlowSlew");
            lua_pushinteger(L, ft_data.BLSchmittInput);
            lua_setfield(L, -2, "BLSchmittInput");
            lua_pushinteger(L, ft_data.BLDriveCurrent);
            lua_setfield(L, -2, "BLDriveCurrent");
            lua_pushinteger(L, ft_data.BHSlowSlew);
            lua_setfield(L, -2, "BHSlowSlew");
            lua_pushinteger(L, ft_data.BHSchmittInput);
            lua_setfield(L, -2, "BHSchmittInput");
            lua_pushinteger(L, ft_data.BHDriveCurrent);
            lua_setfield(L, -2, "BHDriveCurrent");
            lua_pushinteger(L, ft_data.IFAIsFifo7);
            lua_setfield(L, -2, "IFAIsFifo7");
            lua_pushinteger(L, ft_data.IFAIsFifoTar7);
            lua_setfield(L, -2, "IFAIsFifoTar7");
            lua_pushinteger(L, ft_data.IFAIsFastSer7);
            lua_setfield(L, -2, "IFAIsFastSer7");
            lua_pushinteger(L, ft_data.AIsVCP7);
            lua_setfield(L, -2, "AIsVCP7");
            lua_pushinteger(L, ft_data.IFBIsFifo7);
            lua_setfield(L, -2, "IFBIsFifo7");
            lua_pushinteger(L, ft_data.IFBIsFifoTar7);
            lua_setfield(L, -2, "IFBIsFifoTar7");
            lua_pushinteger(L, ft_data.IFBIsFastSer7);
            lua_setfield(L, -2, "IFBIsFastSer7");
            lua_pushinteger(L, ft_data.BIsVCP7);
            lua_setfield(L, -2, "BIsVCP7");
            lua_pushinteger(L, ft_data.PowerSaveEnable);
            lua_setfield(L, -2, "PowerSaveEnable");
            
            // Rev 8 (FT4232H) Extensions
            lua_pushinteger(L, ft_data.PullDownEnable8);
            lua_setfield(L, -2, "PullDownEnable8");
            lua_pushinteger(L, ft_data.SerNumEnable8);
            lua_setfield(L, -2, "SerNumEnable8");
            lua_pushinteger(L, ft_data.ASlowSlew);
            lua_setfield(L, -2, "ASlowSlew");
            lua_pushinteger(L, ft_data.ASchmittInput);
            lua_setfield(L, -2, "ASchmittInput");
            lua_pushinteger(L, ft_data.ADriveCurrent);
            lua_setfield(L, -2, "ADriveCurrent");
            lua_pushinteger(L, ft_data.BSlowSlew);
            lua_setfield(L, -2, "BSlowSlew");
            lua_pushinteger(L, ft_data.BSchmittInput);
            lua_setfield(L, -2, "BSchmittInput");
            lua_pushinteger(L, ft_data.BDriveCurrent);
            lua_setfield(L, -2, "BDriveCurrent");
            lua_pushinteger(L, ft_data.CSlowSlew);
            lua_setfield(L, -2, "CSlowSlew");
            lua_pushinteger(L, ft_data.CSchmittInput);
            lua_setfield(L, -2, "CSchmittInput");
            lua_pushinteger(L, ft_data.CDriveCurrent);
            lua_setfield(L, -2, "CDriveCurrent");
            lua_pushinteger(L, ft_data.DSlowSlew);
            lua_setfield(L, -2, "DSlowSlew");
            lua_pushinteger(L, ft_data.DSchmittInput);
            lua_setfield(L, -2, "DSchmittInput");
            lua_pushinteger(L, ft_data.DDriveCurrent);
            lua_setfield(L, -2, "DDriveCurrent");
            lua_pushinteger(L, ft_data.ARIIsTXDEN);
            lua_setfield(L, -2, "ARIIsTXDEN");
            lua_pushinteger(L, ft_data.BRIIsTXDEN);
            lua_setfield(L, -2, "BRIIsTXDEN");
            lua_pushinteger(L, ft_data.CRIIsTXDEN);
            lua_setfield(L, -2, "CRIIsTXDEN");
            lua_pushinteger(L, ft_data.DRIIsTXDEN);
            lua_setfield(L, -2, "DRIIsTXDEN");
            lua_pushinteger(L, ft_data.AIsVCP8);
            lua_setfield(L, -2, "AIsVCP8");
            lua_pushinteger(L, ft_data.BIsVCP8);
            lua_setfield(L, -2, "BIsVCP8");
            lua_pushinteger(L, ft_data.CIsVCP8);
            lua_setfield(L, -2, "CIsVCP8");
            lua_pushinteger(L, ft_data.DIsVCP8);
            lua_setfield(L, -2, "DIsVCP8");
        }// else lua_pushnil(L);
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Write eeprom.
//   Input: opened device handle, FT_PROGRAM_DATA fields table.
//   Returns: true or false/nil depending on operation success.
static int _ftdi_write_eep(lua_State *L) {
    int result = 0;

    if ((lua_isnumber(L,1)) && (lua_istable(L,2))) { // 1st is handle, 2nd is table
        
        FT_HANDLE handle = (DWORD*)lua_tointeger(L, 1);
        FT_PROGRAM_DATA ft_data;
        char ManufacturerBuf[32];
        char ManufacturerIdBuf[16];
        char DescriptionBuf[64];
        char SerialNumberBuf[16];

        ft_data.Signature1     = 0x00000000;
        ft_data.Signature2     = 0xFFFFFFFF;
        ft_data.Version        = 0x00000004; // EEPROM structure with FT4232H extensions
        ft_data.Manufacturer   = ManufacturerBuf;
        ft_data.ManufacturerId = ManufacturerIdBuf;
        ft_data.Description    = DescriptionBuf;
        ft_data.SerialNumber   = SerialNumberBuf;
        
        FT_STATUS status = FT_EE_Read( handle, &ft_data ); // read EEPROM
        if (status == FT_OK) {

            // fill parameters

            lua_getfield(L, 2, "Manufacturer");
            if (lua_isstring(L, -1)) {
                uint32_t s_len;
                const char *s = lua_tolstring(L, -1, &s_len);
                if (s_len > sizeof(ManufacturerBuf)) s_len = sizeof(ManufacturerBuf);
                memcpy( ManufacturerBuf, s, s_len);
            }
            lua_pop(L, 1);
            lua_getfield(L, 2, "ManufacturerId");
            if (lua_isstring(L, -1)) {
                uint32_t s_len;
                const char *s = lua_tolstring(L, -1, &s_len);
                if (s_len > sizeof(ManufacturerIdBuf)) s_len = sizeof(ManufacturerIdBuf);
                memcpy( ManufacturerIdBuf, s, s_len);
            }
            lua_pop(L, 1);
            lua_getfield(L, 2, "Description");
            if (lua_isstring(L, -1)) {
                uint32_t s_len;
                const char *s = lua_tolstring(L, -1, &s_len);
                if (s_len > sizeof(DescriptionBuf)) s_len = sizeof(DescriptionBuf);
                memcpy( DescriptionBuf, s, s_len);
            }
            lua_pop(L, 1);
            lua_getfield(L, 2, "SerialNumber");
            if (lua_isstring(L, -1)) {
                uint32_t s_len;
                const char *s = lua_tolstring(L, -1, &s_len);
                if (s_len > sizeof(SerialNumberBuf)) s_len = sizeof(SerialNumberBuf);
                memcpy( SerialNumberBuf, s, s_len);
            }
            lua_pop(L, 1);

            lua_getfield(L, 2, "VendorId");
            if (lua_isnumber(L, -1)) ft_data.VendorId = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ProductId");
            if (lua_isnumber(L, -1)) ft_data.ProductId = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "MaxPower");
            if (lua_isnumber(L, -1)) ft_data.MaxPower = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "PnP");
            if (lua_isnumber(L, -1)) ft_data.PnP = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "SelfPowered");
            if (lua_isnumber(L, -1)) ft_data.SelfPowered = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "RemoteWakeup");
            if (lua_isnumber(L, -1)) ft_data.RemoteWakeup = lua_tointeger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "Rev4");
            if (lua_isnumber(L, -1)) ft_data.Rev4 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IsoIn");
            if (lua_isnumber(L, -1)) ft_data.IsoIn = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IsoOut");
            if (lua_isnumber(L, -1)) ft_data.IsoOut = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "PullDownEnable");
            if (lua_isnumber(L, -1)) ft_data.PullDownEnable = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "SerNumEnable");
            if (lua_isnumber(L, -1)) ft_data.SerNumEnable = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "USBVersionEnable");
            if (lua_isnumber(L, -1)) ft_data.USBVersionEnable = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "USBVersion");
            if (lua_isnumber(L, -1)) ft_data.USBVersion = lua_tointeger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "Rev5");
            if (lua_isnumber(L, -1)) ft_data.Rev5 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IsoInA");
            if (lua_isnumber(L, -1)) ft_data.IsoInA = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IsoInB");
            if (lua_isnumber(L, -1)) ft_data.IsoInB = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IsoOutA");
            if (lua_isnumber(L, -1)) ft_data.IsoOutA = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IsoOutB");
            if (lua_isnumber(L, -1)) ft_data.IsoOutB = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "PullDownEnable5");
            if (lua_isnumber(L, -1)) ft_data.PullDownEnable5 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "SerNumEnable5");
            if (lua_isnumber(L, -1)) ft_data.SerNumEnable5 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "USBVersionEnable5");
            if (lua_isnumber(L, -1)) ft_data.USBVersionEnable5 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "USBVersion5");
            if (lua_isnumber(L, -1)) ft_data.USBVersion5 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AIsHighCurrent");
            if (lua_isnumber(L, -1)) ft_data.AIsHighCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BIsHighCurrent");
            if (lua_isnumber(L, -1)) ft_data.BIsHighCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFAIsFifo");
            if (lua_isnumber(L, -1)) ft_data.IFAIsFifo = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFAIsFifoTar");
            if (lua_isnumber(L, -1)) ft_data.IFAIsFifoTar = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFAIsFastSer");
            if (lua_isnumber(L, -1)) ft_data.IFAIsFastSer = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AIsVCP");
            if (lua_isnumber(L, -1)) ft_data.AIsVCP = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFBIsFifo");
            if (lua_isnumber(L, -1)) ft_data.IFBIsFifo = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFBIsFifoTar");
            if (lua_isnumber(L, -1)) ft_data.IFBIsFifoTar = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFBIsFastSer");
            if (lua_isnumber(L, -1)) ft_data.IFBIsFastSer = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BIsVCP");
            if (lua_isnumber(L, -1)) ft_data.BIsVCP = lua_tointeger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "UseExtOsc");
            if (lua_isnumber(L, -1)) ft_data.UseExtOsc = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "HighDriveIOs");
            if (lua_isnumber(L, -1)) ft_data.HighDriveIOs = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "EndpointSize");
            if (lua_isnumber(L, -1)) ft_data.EndpointSize = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "PullDownEnableR");
            if (lua_isnumber(L, -1)) ft_data.PullDownEnableR = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "SerNumEnableR");
            if (lua_isnumber(L, -1)) ft_data.SerNumEnableR = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertTXD");
            if (lua_isnumber(L, -1)) ft_data.InvertTXD = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertRXD");
            if (lua_isnumber(L, -1)) ft_data.InvertRXD = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertRTS");
            if (lua_isnumber(L, -1)) ft_data.InvertRTS = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertCTS");
            if (lua_isnumber(L, -1)) ft_data.InvertCTS = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertDTR");
            if (lua_isnumber(L, -1)) ft_data.InvertDTR = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertDSR");
            if (lua_isnumber(L, -1)) ft_data.InvertDSR = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertDCD");
            if (lua_isnumber(L, -1)) ft_data.InvertDCD = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "InvertRI");
            if (lua_isnumber(L, -1)) ft_data.InvertRI = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "Cbus0");
            if (lua_isnumber(L, -1)) ft_data.Cbus0 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "Cbus1");
            if (lua_isnumber(L, -1)) ft_data.Cbus1 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "Cbus2");
            if (lua_isnumber(L, -1)) ft_data.Cbus2 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "Cbus3");
            if (lua_isnumber(L, -1)) ft_data.Cbus3 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "Cbus4");
            if (lua_isnumber(L, -1)) ft_data.Cbus4 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "RIsD2XX");
            if (lua_isnumber(L, -1)) ft_data.RIsD2XX = lua_tointeger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "PullDownEnable7");
            if (lua_isnumber(L, -1)) ft_data.PullDownEnable7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "SerNumEnable7");
            if (lua_isnumber(L, -1)) ft_data.SerNumEnable7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ALSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.ALSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ALSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.ALSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ALDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.ALDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AHSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.AHSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AHSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.AHSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AHDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.AHDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BLSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.BLSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BLSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.BLSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BLDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.BLDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BHSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.BHSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BHSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.BHSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BHDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.BHDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFAIsFifo7");
            if (lua_isnumber(L, -1)) ft_data.IFAIsFifo7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFAIsFifoTar7");
            if (lua_isnumber(L, -1)) ft_data.IFAIsFifoTar7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFAIsFastSer7");
            if (lua_isnumber(L, -1)) ft_data.IFAIsFastSer7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AIsVCP7");
            if (lua_isnumber(L, -1)) ft_data.AIsVCP7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFBIsFifo7");
            if (lua_isnumber(L, -1)) ft_data.IFBIsFifo7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFBIsFifoTar7");
            if (lua_isnumber(L, -1)) ft_data.IFBIsFifoTar7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "IFBIsFastSer7");
            if (lua_isnumber(L, -1)) ft_data.IFBIsFastSer7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BIsVCP7");
            if (lua_isnumber(L, -1)) ft_data.BIsVCP7 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "PowerSaveEnable");
            if (lua_isnumber(L, -1)) ft_data.PowerSaveEnable = lua_tointeger(L, -1);
            lua_pop(L, 1);

            lua_getfield(L, 2, "PullDownEnable8");
            if (lua_isnumber(L, -1)) ft_data.PullDownEnable8 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "SerNumEnable8");
            if (lua_isnumber(L, -1)) ft_data.SerNumEnable8 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ASlowSlew");
            if (lua_isnumber(L, -1)) ft_data.ASlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ASchmittInput");
            if (lua_isnumber(L, -1)) ft_data.ASchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ADriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.ADriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.BSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.BSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.BDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "CSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.CSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "CSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.CSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "CDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.CDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "DSlowSlew");
            if (lua_isnumber(L, -1)) ft_data.DSlowSlew = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "DSchmittInput");
            if (lua_isnumber(L, -1)) ft_data.DSchmittInput = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "DDriveCurrent");
            if (lua_isnumber(L, -1)) ft_data.DDriveCurrent = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "ARIIsTXDEN");
            if (lua_isnumber(L, -1)) ft_data.ARIIsTXDEN = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BRIIsTXDEN");
            if (lua_isnumber(L, -1)) ft_data.BRIIsTXDEN = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "CRIIsTXDEN");
            if (lua_isnumber(L, -1)) ft_data.CRIIsTXDEN = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "DRIIsTXDEN");
            if (lua_isnumber(L, -1)) ft_data.DRIIsTXDEN = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "AIsVCP8");
            if (lua_isnumber(L, -1)) ft_data.AIsVCP8 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "BIsVCP8");
            if (lua_isnumber(L, -1)) ft_data.BIsVCP8 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "CIsVCP8");
            if (lua_isnumber(L, -1)) ft_data.CIsVCP8 = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_getfield(L, 2, "DIsVCP8");
            if (lua_isnumber(L, -1)) ft_data.DIsVCP8 = lua_tointeger(L, -1);
            lua_pop(L, 1);

            status = FT_EE_Program(handle, &ft_data);
            result = status == FT_OK;
        }
    }
        
    lua_pushboolean( L, result );
    return 1;
}        
#endif


static const luaL_reg lib__ul_ftdi[] = {
    {"ndevs",      _ftdi_ndevs},
    {"devs",       _ftdi_devs},

    {"open",       _ftdi_open},
    {"close",      _ftdi_close},

    {"latency",    _ftdi_latency},
    {"setLatency", _ftdi_setLatency},

    {"availRX",    _ftdi_availRX},
    {"availTX",    _ftdi_availTX},
    {"flushRX",    _ftdi_flushRX},
    {"flushTX",    _ftdi_flushTX},

    {"bm",         _ftdi_bm},
    {"setBm",      _ftdi_setBm},

    {"setBaud",    _ftdi_setBaud},
    {"setDiv",     _ftdi_setDiv},

    {"write",      _ftdi_write},
    {"read",       _ftdi_read},

#if WITH_EEPROM_FUNCTIONS!=0
    {"readEep",    _ftdi_read_eep},
    {"writeEep",   _ftdi_write_eep},
#endif

    {NULL, NULL}
};


LUALIB_API int luaopen__ul_ftdi(lua_State *L) {
    luaL_register(L, "_ul_ftdi", lib__ul_ftdi);
    
    // add CBUS MUX constants
    lua_getglobal(L, "_ul_ftdi");
    lua_newtable(L);

    lua_pushinteger(L, 0);//CBUS_TXDEN);
    lua_setfield(L, -2, "TXDEN");
    lua_pushinteger(L, 1);//CBUS_PWRON);
    lua_setfield(L, -2, "PWRON");
    lua_pushinteger(L, 2);//CBUS_RXLED);
    lua_setfield(L, -2, "RXLED");
    lua_pushinteger(L, 3);//CBUS_TXLED);
    lua_setfield(L, -2, "TXLED");
    lua_pushinteger(L, 4);//CBUS_TXRXLED);
    lua_setfield(L, -2, "TXRXLED");
    lua_pushinteger(L, 5);//CBUS_SLEEP);
    lua_setfield(L, -2, "SLEEP");
    lua_pushinteger(L, 6);//CBUS_CLK48);
    lua_setfield(L, -2, "CLK48");
    lua_pushinteger(L, 7);//CBUS_CLK24);
    lua_setfield(L, -2, "CLK24");
    lua_pushinteger(L, 8);//CBUS_CLK12);
    lua_setfield(L, -2, "CLK12");
    lua_pushinteger(L, 9);//CBUS_CLK6);
    lua_setfield(L, -2, "CLK6");
    lua_pushinteger(L, 10);//CBUS_IOMODE);
    lua_setfield(L, -2, "IOMODE");
    lua_pushinteger(L, 11);//CBUS_BITBANG_WR);
    lua_setfield(L, -2, "BITBANG_WR");
    lua_pushinteger(L, 12);//CBUS_BITBANG_RD);
    lua_setfield(L, -2, "BITBANG_RD");

    lua_setfield(L, -2, "CBUS");

    lua_pop(L, 1);
    
    return 1;
}

