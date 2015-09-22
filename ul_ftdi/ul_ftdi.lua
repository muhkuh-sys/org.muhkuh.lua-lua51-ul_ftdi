--[[ Copyright (c) 2010 god6or@gmail.com under MIT license.

    Communication with FTDI USB-RS232 chips using ftd2xx library.
    
    Uses: ul_time library for RX timeout.
    
    Usage:
        io.ftdi.ndevs() - get number of connected devices
        io.ftdi.devs() - get array of connected devices as Dev objects
        dev = io.ftdi.open(n) - open n-th device (starting from 1)
        dev:latency() - get latency timer (ms)
        dev:latency(2) - set latency timer (ms)
        dev:BM() - get bit mode
        dev:BM(dev.modes.BM_SYNC, 15) - switch to synchronous mode
        dev:BM(0) - reset back to default mode (serial port)
        dev:baud() - get current baud rate
        dev:baud(9600) - set communication speed to 9600x16 Bps
        dev:write(string.rep('U',100)) - send 100 bytes 0x55
        rxNum = dev:availRX() - get count of bytes in receive buffer
        buf = dev:read() - read received bytes
        timeout = dev:waitRX(100, 100); if timeout > 0 then buf = dev:read() end
            - wait for reception of 100 bytes with 10ms timeout.
              If timeout not occured - read received data.
        dev:close() - close device
        
        EEPROM operations:
            dev:readEep() - reads EEPROM and returns parsed table with FT_PROGRAM_DATA fields or
                            nil if operation error.
            dev:writeEep{param=value,} - reads EEPROM, modifies parameters present in table params 
                (look at FT_PROGRAM_DATA structure description in ftd2xx.h),
                writes back FT_PROGRAM_DATA structure.
                Returns true if success, false/nil if fail.
]]


io.ftdi = {
    _ftdi = require'_ul_ftdi',

    -- FTDI devices types strings
    TYPES = {
        [0]="FT232BM", [1]="FT232AM", [2]="FT100AX", [3]="UNKNOWN", [4]="FT2232C",
        [5]="FT232R", [6]="FT2232H", [7]="FT4232H",
        FT232BM=0, FT232AM=1, FT100AX=2, UNKNOWN=3, FT2232C=4, FT232R=5, FT2232H=6, FT4232H=7,
    },

    -- FTDI bit modes
    MODES = {
        BM_RES   = 0,
        BM_ASYNC = 1,
        BM_MPSSE = 2,
        BM_SYNC  = 4,
        BM_HOST  = 8,
        BM_OPTO  = 16,
    },
    
}
io.ftdi.CBUS = io.ftdi._ftdi.CBUS

-- returns number of connected devices
io.ftdi.ndevs = function()
    return io.ftdi._ftdi.ndevs()
end

-- returns array of Dev objects representing connected devices
io.ftdi.devs = function()
    local devs = {}
    local cnt = 0
    for k,v in pairs(io.ftdi._ftdi.devs()) do
        local dev = io.ftdi.Dev:new(v)
        dev.num = cnt
        cnt = cnt + 1
        if dev.t < table.maxn(io.ftdi.TYPES) then dev.ts = io.ftdi.TYPES[dev.t + 1] end
        table.insert(devs, dev)
    end
    return devs
end

-- returns Dev object or opened device (by its number 0..N)
io.ftdi.open = function(num)
    local dev = nil
    if (num >= 1) and (num <= io.ftdi.ndevs()) then
        dev = io.ftdi.devs()[num]
        dev:open()
        if not dev.opened then dev = nil end
    end
    return dev
end

-- FTDI device class
io.ftdi.Dev = {
    num = 0, -- device number
    flags = 0,
    t = 0, -- type
    id = 0, -- ID
    lid = 0,
    handle = 0, -- handle
    serial = '',
    descr = '',
    opened = false, -- true if device opened
    rxTOut = 10000, -- RX timeout (in 100us increments)

    types = io.ftdi.types,
    modes = io.ftdi.modes,

    del_us = os.delay_us,
    
    -- create device instance
    new = function(self, o)
        local p = o or {}
        setmetatable(p, self)
        self.__index = self
        return p
    end,

    -- open device
    open = function(self)
        if not self.opened then
            local result = io.ftdi._ftdi.open(self.num)
            if result ~= nil then
                self.handle = result
                self.opened = true
            end
        end
        return self.opened
    end,

    -- close device
    close = function(self)
        if self.opened then
            if io.ftdi._ftdi.close(self.handle) then self.opened = false end
        end
        return not self.opened
    end,

    -- flush RX queue
    flushRX = function(self)
        if self.opened then io.ftdi._ftdi.flushRX(self.handle) end
        return self
    end,
    -- flush TX queue
    flushTX = function(self)
        if self.opened then io.ftdi._ftdi.flushTX(self.handle) end
        return self
    end,
    flush = function(self)
        self:flushRX():flushTX()
        return self
    end,

    -- get/set latency timer
    latency = function(self, latency)
        if not latency then return io.ftdi._ftdi.latency(self.handle)
        else
            if self.opened then io.ftdi._ftdi.setLatency(self.handle, latency) end
            return self
        end
    end,

    -- get/set/reset bit mode. If bm == -1, resets bit mode
    BM = function(self, bm, mask)
        if not bm then return io.ftdi._ftdi.bm(self.handle)
        else
            if not mask then mask = 0 end
            if self.opened then io.ftdi._ftdi.setBm(self.handle, bm, mask) end
            return self
        end
    end,

    -- set baud rate
    baud = function(self, baud)
        if self.opened then io.ftdi._ftdi.setBaud(self.handle, baud) end
        return self
    end,
    
    -- set baud rate divisor
    div = function(self, div)
        if self.opened then io.ftdi._ftdi.setDiv(self.handle, div) end
        return self
    end,

    -- get number of bytes in RX queue
    availRX = function(self)
        if self.opened then return io.ftdi._ftdi.availRX(self.handle)
        else return nil end
    end,
    -- get number of bytes in TX queue
    availTX = function(self)
        if self.opened then return io.ftdi._ftdi.availTX(self.handle)
        else return nil end
    end,

    -- read data
    read = function(self)
        if self.opened then return io.ftdi._ftdi.read(self.handle)
        else return "" end
    end,
    -- Wait until reception of >=nb bytes, tout - timeout time in 100us increments.
    -- Returns remaining timeout counter, == 0 if timeout achieved.
    waitRX = function(self, nb, tout)
        if not tout then tout = self.rxTOut end
        while (self:availRX() < nb) and (tout > 0) do
            self.del_us(100)
            tout = tout - 1
        end
        return tout
    end,

    -- write data
    write = function(self, data)
        if self.opened then io.ftdi._ftdi.write(self.handle, data) end
        return self
    end,

    -- read EEPROM contents into table
    readEep = function(self)
        return io.ftdi._ftdi.readEep(self.handle)
    end,
    -- write EEPROM parameters from table
    writeEep = function(self, eeTable)
        return io.ftdi._ftdi.writeEep(self.handle, eeTable)
    end,
}

