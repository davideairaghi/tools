
-- CUSTOMIZE BELOW ACCORDING TO YOUR NEEDINGS

-- set this to your executable full path
local skygofullpath = "C:\\Users\\davide\\Desktop\\skygo.bat";
-- set to the real executable file name
local skygoexe      = "Sky Go.exe";

-- set to your screen width
local screen_w = 1920;
-- set to your screen height
local screen_h = 1080;

-- set to play icon center for first video in movie detail page
local btn_playmovie_x = 525;
local btn_playmovie_y = 510;
-- set to play/pause icon center (window already maximized)
local btn_play_x = 64;
local btn_play_y = 1034;
-- set to stop icon center (window already maximized)
local btn_stop_x = 1873;
local btn_stop_y = 48;
-- set to volume up icon center (window already maximized)
local btn_volup_x = 0; -- unused
local btn_volup_y = 0; -- unused
-- set to volume down icon center (window already maximized)
local btn_voldown_x = 0; -- unused
local btn_voldown_y = 0; -- unused
-- set to full screen icon center (window not maximized)
local btn_fullscreen_x = 1859;
local btn_fullscreen_y = 991;
-- set to normal screen icon center (window already maximized)
local btn_stdscreen_x  = 1870;
local btb_stdscreen_y  = 1042;

-- BELOW HERE DO NOT CHANGE ANYTHING 

local WM_SYSCOMMAND = 0x0112;
local SC_MAXIMIZE   = 0xF030;
local SC_CLOSE      = 0xF060;

local SCREEN_STD = 0;
local SCREEN_MAX = 1;

local screen_status    = SCREEN_STD;
local skygo_loaded     = false;
local layer_status     = SCREEN_STD;

local keyboard = libs.keyboard;
local mouse    = libs.mouse;
local win      = libs.win;
local device   = libs.device;

function setupwindow()
	if OS_WINDOWS then
		local hwnd = win.window(skygoexe);
		if (hwnd == 0) then 
			return false; 
		end
		win.switchto(hwnd);
		hwnd = win.active();
		skygo_loaded = true;
		if (screen_status ~= SCREEN_MAX) then
			win.send(hwnd,WM_SYSCOMMAND,SC_MAXIMIZE,0);
			screen_status = SCREEN_MAX;
		end
		return true;
	else
		return false;
	end
end

function setuplayer()
	if OS_WINDOWS and skygo_loaded then	
		if (layer_status == SCREEN_MAX) then return true; end
		mouse.moveto(btn_fullscreen_x,btn_fullscreen_y);
		mouse.click();
		layer_status = SCREEN_MAX;
		os.sleep(1500);
		local hwnd = win.window(skygoexe);
		win.switchto(hwnd);
		return true;
	else
		return false
	end
end

actions.launch = function()
	if OS_WINDOWS then
		local hwnd = win.window(skygoexe);
		if (hwnd == 0) then os.start(skygofullpath); end
		os.sleep(30000);
		win.switchtowait(skygoexe,15000);
		setupwindow();
	end
end

actions.close = function()
	if OS_WINDOWS then
		local hwnd = win.window(skygoexe);
		if (hwnd ~= 0) then
			win.switchto(hwnd)
			win.send(hwnd,WM_SYSCOMMAND,SC_CLOSE,0);
			skygo_loaded = false;
			screen_status = SCREEN_STD;
			layer_status  = SCREEN_STD;
		end
	end
end

actions.playmovie = function()
	if OS_WINDOWS then
		if (not setupwindow()) then return; end
		mouse.moveto(btn_playmovie_x,btn_playmovie_y);
		os.sleep(1000);
		mouse.click();
		os.sleep(30000);
		setuplayer();
	end
end

actions.playpause = function()
	if OS_WINDOWS and skygo_loaded then
	if (not setupwindow()) then return; end
		if (not setuplayer()) then return; end
		mouse.moveto(btn_play_x,btn_play_y);
		mouse.click();
	end
end

actions.stop = function()
	if OS_WINDOWS and skygo_loaded then
		if (not setupwindow()) then return; end
		mouse.moveto(btn_stop_x,btn_stop_y);
		mouse.click();
		layer_status = SCREEN_STD;
	end
end

actions.volup = function()
	if OS_WINDOWS and skygo_loaded then
		if (not setupwindow()) then return; end
		if (not setuplayer()) then return; end
		mouse.moveto(btn_volup_x,btn_volup_y);
		mouse.click();
	end
end

actions.voldown = function()
	if OS_WINDOWS and skygo_loaded then
		if (not setupwindow()) then return; end
		if (not setuplayer()) then return; end
		mouse.moveto(btn_voldown_x,btn_down_y);
		mouse.click();
	end
end