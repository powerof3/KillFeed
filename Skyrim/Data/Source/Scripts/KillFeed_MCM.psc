Scriptname KillFeed_MCM extends MCM_ConfigBase

Function OnConfigOpen() Native
Function OnConfigClose() Native
Int Function ColorStringToInt(String sStr) Native
Function LoadMCMCategorySettings() Native
Function LoadMCMScaledSettings() Native
Function LoadMCMGenericSettings() Native

Function UpdateStringWithColor(String sSetting)

	string colorSetting = "r" + sSetting
	string stringSetting = "s" + sSetting

	int color = GetModSettingInt(colorSetting)

	int red = ColorComponent.GetRed(color)
	int green = ColorComponent.GetGreen(color)
	int blue = ColorComponent.GetBlue(color)

	string colorStr = red + "," + green + "," + blue
	SetModSettingString(stringSetting, colorStr)
	RefreshMenu()

endFunction

Function UpdateColorWithString(String sSetting)

	string colorSetting = "r" + sSetting
	string stringSetting = "s" + sSetting

	string colorString = GetModSettingString(stringSetting)
	
	int color = ColorStringToInt(colorString)	
	SetModSettingInt(colorSetting, color)
	
	RefreshMenu()

endFunction

Function UpdateTextColors(String section)

	UpdateColorWithString("TextColor:"+section)

endFunction

Event OnConfigInit()

	UpdateTextColors("Player")
	UpdateTextColors("Followers")
	UpdateTextColors("Allies")
	UpdateTextColors("Hostiles")
	UpdateTextColors("Text")

	UpdateColorWithString("IconTint:Icons")
	UpdateColorWithString("BgColor:Background")
	
endEvent

Event OnGameReload()

	parent.OnGameReload()
	
	OnConfigInit()
	
endEvent

Event OnSettingChange(String a_ID)

	if a_ID ==  "rTextColor:Player"
		UpdateStringWithColor("TextColor:Player")
		LoadMCMCategorySettings();
	elseif a_ID ==  "rTextColor:Followers"
		UpdateStringWithColor("TextColor:Followers")
		LoadMCMCategorySettings();
	elseif a_ID ==  "rTextColor:Allies"
		UpdateStringWithColor("TextColor:Allies")
		LoadMCMCategorySettings();
	elseif a_ID ==  "rTextColor:Hostiles"
		UpdateStringWithColor("TextColor:Hostiles")
		LoadMCMCategorySettings();
	elseif a_ID ==  "rTextColor:Text"
		UpdateStringWithColor("TextColor:Text")
		LoadMCMCategorySettings();
	;	
	elseif a_ID == "sTextColor:Player"
		UpdateColorWithString("TextColor:Player")
		LoadMCMCategorySettings();
	elseif a_ID == "sTextColor:Followers"
		UpdateColorWithString("TextColor:Followers")
		LoadMCMCategorySettings();
	elseif a_ID == "sTextColor:Allies"
		UpdateColorWithString("TextColor:Allies")
		LoadMCMCategorySettings();
	elseif a_ID == "sTextColor:Hostiles"
		UpdateColorWithString("TextColor:Hostiles")
		LoadMCMCategorySettings();
	elseif a_ID ==  "sTextColor:Text"
		UpdateColorWithString("TextColor:Text")
		LoadMCMCategorySettings();

	;
	elseif a_ID ==  "rIconTint:Icons"
		UpdateStringWithColor("IconTint:Icons")
		LoadMCMCategorySettings();
	elseif a_ID ==  "sIconTint:Icons"
		UpdateColorWithString("IconTint:Icons")
		LoadMCMCategorySettings();
	;
	elseif a_ID ==  "rBgColor:Background"
		UpdateStringWithColor("BgColor:Background")
		LoadMCMCategorySettings();
	elseif a_ID ==  "sBgColor:Background"
		UpdateColorWithString("BgColor:Background")
		LoadMCMCategorySettings();
	;
	elseif a_ID == "fTextSize:Text" || a_ID == "fWidth:Window" || a_ID == "fHeight:Window" || a_ID == "fPosX:Window" || a_ID == "fPosY:Window" || a_ID == "fSlideOffsetX:Animation" || a_ID == "fSlideOffsetY:Animation"
		LoadMCMScaledSettings();
		RefreshMenu()
	else
		LoadMCMGenericSettings()
	endif
		
endEvent