#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" 
#include "Interfaces.h"
#include "CRC32.h"

#define WINDOW_WIDTH 950
#define WINDOW_HEIGHT 630

aristoispejawindow Menu::Window;

void KnifeApplyCallbk()
{
	static ConVar* Meme = Interfaces::CVar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
}

void SaveCallbk()
{
	switch (Menu::Window.ConfigBox.GetIndex())
	{
	case 0:
		GUI.SaveWindowState(&Menu::Window, "legit.cfg");
		break;
	case 1:
		GUI.SaveWindowState(&Menu::Window, "hvh.cfg");
		break;
	case 2:
		GUI.SaveWindowState(&Menu::Window, "Other.cfg");
		break;
	}
}

void LoadCallbk()
{
	switch (Menu::Window.ConfigBox.GetIndex())
	{
	case 0:
		GUI.LoadWindowState(&Menu::Window, "legit.cfg");
		break;
	case 1:
		GUI.LoadWindowState(&Menu::Window, "hvh.cfg");
		break;
	case 2:
		GUI.LoadWindowState(&Menu::Window, "Other.cfg");
		break;
	}
}


void UnLoadCallbk()
{
	DoUnload = true;
}

void aristoispejawindow::Setup()
{
	SetPosition(350, 50);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("SchoolShooter");

	RegisterTab(&RageBotTab);
	RegisterTab(&LegitBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&SettingsTab);
	RegisterTab(&MiscTab);
	RegisterTab(&ColorsTab);

	RECT Client = GetClientArea();
	Client.bottom -= 29;

	RageBotTab.Setup();
	LegitBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();
	SettingsTab.Setup();
	ColorsTab.Setup();

#pragma region Bottom Buttons

	ConfigBox.SetFileId("cfg_box");
	ConfigBox.AddItem("Legit");
	ConfigBox.AddItem("HvH");
	ConfigBox.AddItem("Other");
	ConfigBox.SetSize(112, 200);
	ConfigBox.SetPosition(10, 410); // +350

	SaveButton.SetText("Save");
	SaveButton.SetCallback(SaveCallbk);
	SaveButton.SetSize(112, 200);
	SaveButton.SetPosition(10, 430);

	LoadButton.SetText("Load");
	LoadButton.SetCallback(LoadCallbk);
	LoadButton.SetSize(112, 200);
	LoadButton.SetPosition(10, 460);

	MiscTab.RegisterControl(&SaveButton);
	MiscTab.RegisterControl(&LoadButton);
	MiscTab.RegisterControl(&ConfigBox);

	LegitBotTab.RegisterControl(&SaveButton);
	LegitBotTab.RegisterControl(&LoadButton);
	LegitBotTab.RegisterControl(&ConfigBox);

	RageBotTab.RegisterControl(&SaveButton);
	RageBotTab.RegisterControl(&LoadButton);
	RageBotTab.RegisterControl(&ConfigBox);

	VisualsTab.RegisterControl(&SaveButton);
	VisualsTab.RegisterControl(&LoadButton);
	VisualsTab.RegisterControl(&ConfigBox);

	ColorsTab.RegisterControl(&SaveButton);
	ColorsTab.RegisterControl(&LoadButton);
	ColorsTab.RegisterControl(&ConfigBox);


	SettingsTab.RegisterControl(&SaveButton);
	SettingsTab.RegisterControl(&LoadButton);
	SettingsTab.RegisterControl(&ConfigBox);

#pragma endregion
}

void CRageBotTab::Setup()
{
	SetTitle("RAGE");

	ActiveLabel.SetPosition(46 + 134, -15 -8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(86 + 134, -15 -8);
	RegisterControl(&Active);
	
#pragma region Aimbot

	AimbotGroup.SetPosition(16 + 134, -3);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(360, 230);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceCheckBox("Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceCheckBox("Auto Fire", this, &AimbotAutoFire);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(39.f);
	AimbotGroup.PlaceOtherControl("FOV Range", this, &AimbotFov);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceCheckBox("Perfect Silent Aim", this, &AimbotSilentAim);

	AimbotAimStep.SetFileId("aim_aimstep");
	AimbotGroup.PlaceCheckBox("Casual Rage Mode", this, &AimbotAimStep);

	AccuracyBacktracking.SetFileId("acc_bcktrk");
	AimbotGroup.PlaceCheckBox("Backtrack", this, &AccuracyBacktracking);

	AWPAtBody.SetFileId("aim_awpatbody");
	AimbotGroup.PlaceCheckBox("AWP Body Aim", this, &AWPAtBody);

	AutoRevolver.SetFileId("aim_autorevolver");
	AimbotGroup.PlaceCheckBox("Auto Revolver", this, &AutoRevolver);

	//AimbotKeyPress.SetFileId("aim_usekey");
	//AimbotGroup.PlaceCheckBox("On Key Press", this, &AimbotKeyPress);

	//AimbotKeyBind.SetFileId("aim_key");
	//AimbotGroup.PlaceOtherControl("Key", this, &AimbotKeyBind);
#pragma endregion Aimbot Controls Get Setup in here

#pragma region Accuracy
	AccuracyGroup.SetPosition(408 + 120, 257 - 8);
	AccuracyGroup.SetText("Accuracy");
	AccuracyGroup.SetSize(360, 245);
	RegisterControl(&AccuracyGroup);

	AccuracyRecoil.SetFileId("acc_norecoil");
	AccuracyGroup.PlaceCheckBox("Remove Recoil", this, &AccuracyRecoil);

	AccuracyAutoWall.SetFileId("acc_awall");
	AccuracyGroup.PlaceCheckBox("Penetrate Walls", this, &AccuracyAutoWall);

	AccuracyMinimumDamage.SetFileId("acc_mindmg");
	AccuracyMinimumDamage.SetBoundaries(1.f, 99.f);
	AccuracyMinimumDamage.SetValue(1.f);
	AccuracyGroup.PlaceOtherControl("Minimum Damage", this, &AccuracyMinimumDamage);

	AccuracyAutoScope.SetFileId("acc_scope");
	AccuracyGroup.PlaceCheckBox("Auto Scope", this, &AccuracyAutoScope);

	AdvancedResolver.SetFileId("acc_advancedresolver");
	AccuracyGroup.PlaceCheckBox("Pitch Resolver", this, &AdvancedResolver);

	AimbotResolver.SetFileId("acc_aaa");
	AimbotResolver.AddItem("Off");
	AimbotResolver.AddItem("Lv1");
	AimbotResolver.AddItem("Lv2");
	AimbotResolver.AddItem("Meme1");
	AimbotResolver.AddItem("Meme2");
	AimbotResolver.AddItem("Meme3");
	AimbotResolver.AddItem("Ok");
	AimbotResolver.AddItem("Good");
	AimbotResolver.AddItem("?");
	AimbotResolver.AddItem("Meme4");
	AimbotResolver.AddItem("Meme5");
	AccuracyGroup.PlaceOtherControl("Yaw Resolver", this, &AimbotResolver);

	AimbotBaimOnKey.SetFileId("baimonkeypress");
	AccuracyGroup.PlaceOtherControl("Body Aim", this, &AimbotBaimOnKey);

	AccuracyHitchance.SetFileId("acc_chance");
	AccuracyHitchance.SetBoundaries(0, 100);
	AccuracyHitchance.SetValue(0);
	AccuracyGroup.PlaceOtherControl("Hitchance", this, &AccuracyHitchance);

	BaimIfUnderXHealth.SetFileId("acc_BaimIfUnderXHealth");
	BaimIfUnderXHealth.SetBoundaries(0, 100);
	BaimIfUnderXHealth.SetValue(0);
	AccuracyGroup.PlaceOtherControl("Body-Aim if HP is under X", this, &BaimIfUnderXHealth);

#pragma endregion  Accuracy controls get Setup in here

#pragma region Target
	TargetGroup.SetPosition(16 + 130, 257 - 8);
	TargetGroup.SetText("Target");
	TargetGroup.SetSize(360, 245);
	RegisterControl(&TargetGroup);

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("Closest To Crosshair");
	TargetSelection.AddItem("Distance");
	TargetSelection.AddItem("Lowest Health");
	TargetSelection.AddItem("Threat");
	TargetSelection.AddItem("Next Shot");
	TargetGroup.PlaceOtherControl("Selection", this, &TargetSelection);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("None");
	TargetHitbox.AddItem("Head");
	TargetHitbox.AddItem("Neck");
	TargetHitbox.AddItem("Chest");
	TargetHitbox.AddItem("Stomach");
	TargetHitbox.AddItem("All Low");
	TargetHitbox.AddItem("All Medium");
	TargetHitbox.AddItem("All High");
	TargetGroup.PlaceOtherControl("Hitbox", this, &TargetHitbox);

	TargetMultipoint.SetFileId("tgt_multipoint");
	TargetGroup.PlaceCheckBox("Multipoint", this, &TargetMultipoint);

	TargetPointscale.SetFileId("tgt_pointscale");
	TargetPointscale.SetBoundaries(0.f, 10.f);
	TargetPointscale.SetValue(5.f);
	TargetGroup.PlaceOtherControl("Pointscale", this, &TargetPointscale);
#pragma endregion Targetting controls 

#pragma region AntiAim
	AntiAimGroup.SetPosition(408 + 120, -15 -8);
	AntiAimGroup.SetText("Anti Aim");
	AntiAimGroup.SetSize(360, 248);
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceCheckBox("Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("Off");
	AntiAimPitch.AddItem("Halfdown");
	AntiAimPitch.AddItem("Jitter");
	AntiAimPitch.AddItem("Down");
	AntiAimPitch.AddItem("Up");
	AntiAimPitch.AddItem("Zero");
	AntiAimPitch.AddItem("upsidedown");
	AntiAimPitch.AddItem("upsidedown2");
	AntiAimPitch.AddItem("upsidedownjitter");
	AntiAimGroup.PlaceOtherControl("Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("Off");
	AntiAimYaw.AddItem("Fast Spin");
	AntiAimYaw.AddItem("Slow Spin");
	AntiAimYaw.AddItem("Jitter");
	AntiAimYaw.AddItem("180 Jitter");
	AntiAimYaw.AddItem("Backwards");
	AntiAimYaw.AddItem("Backwards Jitter");
	AntiAimYaw.AddItem("Sideways Switch");
	AntiAimYaw.AddItem("Sideways Right");
	AntiAimYaw.AddItem("Sideways Left");
	AntiAimYaw.AddItem("LBY");
	AntiAimYaw.AddItem("LBY Jitter");
	AntiAimYaw.AddItem("LBY Sideways");
	AntiAimYaw.AddItem("LBY Spin");
	AntiAimYaw.AddItem("FakeJitterTest");
	AntiAimYaw.AddItem("FakeJitterMoveTest");
	AntiAimYaw.AddItem("Memes");
	AntiAimYaw.AddItem("Switch");
	AntiAimYaw.AddItem("adaptive");
	AntiAimYaw.AddItem("anticorrection");
	AntiAimYaw.AddItem("FakeLower");
	AntiAimYaw.AddItem("Break");
	AntiAimYaw.AddItem("Fake1");
	AntiAimYaw.AddItem("Fake2");
	AntiAimYaw.AddItem("Fake3");
	AntiAimYaw.AddItem("Lby180S");
	AntiAimYaw.AddItem("LbyAA");
	AntiAimGroup.PlaceOtherControl("Real Yaw", this, &AntiAimYaw);

	FakeYaw.SetFileId("fakeaa");
	FakeYaw.AddItem("Off");
	FakeYaw.AddItem("Fast Spin");
	FakeYaw.AddItem("Slow Spin");
	FakeYaw.AddItem("Jitter");
	FakeYaw.AddItem("180 Jitter");
	FakeYaw.AddItem("Backwards");
	FakeYaw.AddItem("Backwards Jitter");
	FakeYaw.AddItem("Sideways Switch");
	FakeYaw.AddItem("Sideways Right");
	FakeYaw.AddItem("Sideways Left");
	FakeYaw.AddItem("LBY");
	FakeYaw.AddItem("LBY Jitter");
	FakeYaw.AddItem("LBY Sideways");
	FakeYaw.AddItem("LBY Spin");
	FakeYaw.AddItem("Switch");
	FakeYaw.AddItem("adaptive");
	FakeYaw.AddItem("Anticorrection");
	FakeYaw.AddItem("FakeLower");
	FakeYaw.AddItem("Break");
	FakeYaw.AddItem("Fake1");
	FakeYaw.AddItem("Fake2");
	FakeYaw.AddItem("Fake3");
	FakeYaw.AddItem("FakeLbyAA");
	AntiAimGroup.PlaceOtherControl("Fake Yaw", this, &FakeYaw);

	MoveYaw.SetFileId("aa_y_moving");
	MoveYaw.AddItem("Off");
	MoveYaw.AddItem("Fast Spin");
	MoveYaw.AddItem("Slow Spin");
	MoveYaw.AddItem("Jitter");
	MoveYaw.AddItem("180 Jitter");
	MoveYaw.AddItem("Backwards");
	MoveYaw.AddItem("Backwards Jitter");
	MoveYaw.AddItem("Sideways Switch");
	MoveYaw.AddItem("Sideways Right");
	MoveYaw.AddItem("Sideways Left");
	MoveYaw.AddItem("LBY");
	MoveYaw.AddItem("LBY Jitter");
	MoveYaw.AddItem("LBY Sideways");
	MoveYaw.AddItem("LBY Spin");
	MoveYaw.AddItem("Same");
	AntiAimGroup.PlaceOtherControl("Moving Yaw", this, &MoveYaw);

	MoveYawFake.SetFileId("movingfakeaa");
	MoveYawFake.AddItem("Off");
	MoveYawFake.AddItem("Fast Spin");
	MoveYawFake.AddItem("Slow Spin");
	MoveYawFake.AddItem("Jitter");
	MoveYawFake.AddItem("180 Jitter");
	MoveYawFake.AddItem("Backwards");
	MoveYawFake.AddItem("Backwards Jitter");
	MoveYawFake.AddItem("Sideways Switch");
	MoveYawFake.AddItem("Sideways Right");
	MoveYawFake.AddItem("Sideways Left");
	MoveYawFake.AddItem("LBY");
	MoveYawFake.AddItem("LBY Jitter");
	MoveYawFake.AddItem("LBY Sideways");
	MoveYawFake.AddItem("LBY Spin");
	MoveYawFake.AddItem("Same");
	AntiAimGroup.PlaceOtherControl("Moving Yaw Fake", this, &MoveYawFake);

	BreakLBY.SetFileId("aa_break");
	BreakLBY.AddItem("Off");
	BreakLBY.AddItem("45�");
	BreakLBY.AddItem("90�");
	BreakLBY.AddItem("180�");
	BreakLBY.AddItem("FakeAngle");
	AntiAimGroup.PlaceOtherControl("LBY Breaker Angle", this, &BreakLBY);

	FlipAA.SetFileId("aa_flip");
	AntiAimGroup.PlaceCheckBox("Flip AA on Hit", this, &FlipAA);

	AntiAimTarget.SetFileId("aa_target");
	AntiAimGroup.PlaceCheckBox("At Targets", this, &AntiAimTarget);

	FlipKey.SetFileId("aim_key");
	AntiAimGroup.PlaceOtherControl("Flip Key", this, &FlipKey);
#pragma endregion  AntiAim controls get setup in here
}

void CLegitBotTab::Setup()
{
	SetTitle("LEGIT");

	ActiveLabel.SetPosition(46 + 134, -15 - 8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(86 + 134, -15 - 8);
	RegisterControl(&Active);

#pragma region Aimbot
	AimbotGroup.SetPosition(16 + 134, -3);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(240, 220);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceCheckBox("Enable", this, &AimbotEnable);

	AimbotBacktrack.SetFileId("legit_backtrack");
	AimbotGroup.PlaceCheckBox("Backtrack Ticks", this, &AimbotBacktrack);

	TickModulation.SetFileId("tick_modulate");
	TickModulation.SetBoundaries(0.1f, 13.f);
	TickModulation.SetValue(13.f);
	AimbotGroup.PlaceOtherControl("Tick Modulation", this, &TickModulation);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceCheckBox("Auto Fire", this, &AimbotAutoFire);

	AimbotFriendlyFire.SetFileId("aim_friendfire");
	AimbotGroup.PlaceCheckBox("Friendly Fire", this, &AimbotFriendlyFire);

	AimbotSmokeCheck.SetFileId("otr_smokecheck");
	AimbotGroup.PlaceCheckBox("Smoke Check", this, &AimbotSmokeCheck);

	AimbotKeyPress.SetFileId("aim_usekey");
	AimbotGroup.PlaceCheckBox("On Key", this, &AimbotKeyPress);

	AimbotKeyBind.SetFileId("aim_key");
	AimbotGroup.PlaceOtherControl("Key Bind", this, &AimbotKeyBind);

#pragma endregion Aimbot shit

#pragma region Triggerbot
	TriggerGroup.SetPosition(272 + 134, -15 - 8);
	TriggerGroup.SetText("Triggerbot");
	TriggerGroup.SetSize(240, 220);
	RegisterControl(&TriggerGroup);

	TriggerEnable.SetFileId("trig_enable");
	TriggerGroup.PlaceCheckBox("Enable", this, &TriggerEnable);

	TriggerKeyPress.SetFileId("trig_onkey");
	TriggerGroup.PlaceCheckBox("On Key Press", this, &TriggerKeyPress);

	TriggerKeyBind.SetFileId("trig_key");
	TriggerGroup.PlaceOtherControl("Key Bind", this, &TriggerKeyBind);

	TriggerRecoil.SetFileId("trig_recoil");
	TriggerGroup.PlaceCheckBox("Recoil Control", this, &TriggerRecoil);

	TriggerSmokeCheck.SetFileId("trig_smokecheck");
	TriggerGroup.PlaceCheckBox("Smoke Check", this, &TriggerSmokeCheck);

	TriggerDelay.SetFileId("trig_time");
	TriggerDelay.SetBoundaries(0, 100);
	TriggerDelay.SetValue(1);
	TriggerGroup.PlaceOtherControl("Delay", this, &TriggerDelay);

#pragma endregion Triggerbot stuff

#pragma region TriggerbotFilter
	TriggerFilterGroup.SetPosition(528 + 134, -15 - 8);
	TriggerFilterGroup.SetText("Triggerbot Filter");
	TriggerFilterGroup.SetSize(240, 220);
	RegisterControl(&TriggerFilterGroup);

	TriggerHead.SetFileId("trig_head");
	TriggerFilterGroup.PlaceCheckBox("Head", this, &TriggerHead);

	TriggerChest.SetFileId("trig_chest");
	TriggerFilterGroup.PlaceCheckBox("Chest", this, &TriggerChest);

	TriggerStomach.SetFileId("trig_stomach");
	TriggerFilterGroup.PlaceCheckBox("Stomach", this, &TriggerStomach);

	TriggerArms.SetFileId("trig_arms");
	TriggerFilterGroup.PlaceCheckBox("Arms", this, &TriggerArms);

	TriggerLegs.SetFileId("trig_legs");
	TriggerFilterGroup.PlaceCheckBox("Legs", this, &TriggerLegs);

	TriggerTeammates.SetFileId("trig_teammates");
	TriggerFilterGroup.PlaceCheckBox("Friendly Fire", this, &TriggerTeammates);

#pragma endregion TriggerbotFilter stuff

#pragma region Main Weapon
	WeaponMainGroup.SetPosition(16 + 134, 235);
	WeaponMainGroup.SetText("Rifles");
	WeaponMainGroup.SetSize(240, 135);
	RegisterControl(&WeaponMainGroup);

	WeaponMainHitbox.SetFileId("main_hitbox");
	WeaponMainHitbox.AddItem("Head");
	WeaponMainHitbox.AddItem("Neck");
	WeaponMainHitbox.AddItem("Chest");
	WeaponMainHitbox.AddItem("Stomach");
	WeaponMainHitbox.AddItem("Multihitbox");
	WeaponMainGroup.PlaceOtherControl("Hitbox", this, &WeaponMainHitbox);

	WeaponMainSpeed.SetFileId("main_speed");
	WeaponMainSpeed.SetBoundaries(0.f, 100.f);
	WeaponMainSpeed.SetValue(1.f);
	WeaponMainGroup.PlaceOtherControl("Max Speed", this, &WeaponMainSpeed);

	WeaponMainFoV.SetFileId("main_fov");
	WeaponMainFoV.SetBoundaries(0.f, 30.f);
	WeaponMainFoV.SetValue(3.5f);
	WeaponMainGroup.PlaceOtherControl("Field of View", this, &WeaponMainFoV);

	WeaponMainRecoil.SetFileId("main_recoil");
	WeaponMainRecoil.SetBoundaries(0.f, 2.f);
	WeaponMainRecoil.SetValue(1.f);
	WeaponMainGroup.PlaceOtherControl("Recoil Control", this, &WeaponMainRecoil);

	WeaponMainAimtime.SetValue(0);
	WeaoponMainStartAimtime.SetValue(0);

#pragma endregion

#pragma region Pistols
	WeaponPistGroup.SetPosition(272 + 134, 220);
	WeaponPistGroup.SetText("Pistols");
	WeaponPistGroup.SetSize(240, 135);
	RegisterControl(&WeaponPistGroup);

	WeaponPistHitbox.SetFileId("pist_hitbox");
	WeaponPistHitbox.AddItem("Head");
	WeaponPistHitbox.AddItem("Neck");
	WeaponPistHitbox.AddItem("Chest");
	WeaponPistHitbox.AddItem("Stomach");
	WeaponPistHitbox.AddItem("Multihitbox");
	WeaponPistGroup.PlaceOtherControl("Hitbox", this, &WeaponPistHitbox);

	WeaponPistSpeed.SetFileId("pist_speed");
	WeaponPistSpeed.SetBoundaries(0.f, 100.f);
	WeaponPistSpeed.SetValue(1.0f);
	WeaponPistGroup.PlaceOtherControl("Max Speed", this, &WeaponPistSpeed);

	WeaponPistFoV.SetFileId("pist_fov");
	WeaponPistFoV.SetBoundaries(0.f, 30.f);
	WeaponPistFoV.SetValue(3.f);
	WeaponPistGroup.PlaceOtherControl("Field of View", this, &WeaponPistFoV);

	WeaponPistRecoil.SetFileId("pist_recoil");
	WeaponPistRecoil.SetBoundaries(0.f, 2.f);
	WeaponPistRecoil.SetValue(1.f);
	WeaponPistGroup.PlaceOtherControl("Recoil Control", this, &WeaponPistRecoil);

	WeaponPistAimtime.SetValue(0);
	WeaoponPistStartAimtime.SetValue(0);

#pragma endregion

#pragma region Snipers
	WeaponSnipGroup.SetPosition(528 + 134, 220);
	WeaponSnipGroup.SetText("Snipers");
	WeaponSnipGroup.SetSize(240, 135);
	RegisterControl(&WeaponSnipGroup);

	WeaponSnipHitbox.SetFileId("snip_hitbox");
	WeaponSnipHitbox.AddItem("Head");
	WeaponSnipHitbox.AddItem("Neck");
	WeaponSnipHitbox.AddItem("Chest");
	WeaponSnipHitbox.AddItem("Stomach");
	WeaponSnipHitbox.AddItem("Multihitbox");
	WeaponSnipGroup.PlaceOtherControl("Hitbox", this, &WeaponSnipHitbox);

	WeaponSnipSpeed.SetFileId("snip_speed");
	WeaponSnipSpeed.SetBoundaries(0.f, 100.f);
	WeaponSnipSpeed.SetValue(1.5f);
	WeaponSnipGroup.PlaceOtherControl("Max Speed", this, &WeaponSnipSpeed);

	WeaponSnipFoV.SetFileId("snip_fov");
	WeaponSnipFoV.SetBoundaries(0.f, 30.f);
	WeaponSnipFoV.SetValue(2.f);
	WeaponSnipGroup.PlaceOtherControl("Field of View", this, &WeaponSnipFoV);

	WeaponSnipRecoil.SetFileId("snip_recoil");
	WeaponSnipRecoil.SetBoundaries(0.f, 2.f);
	WeaponSnipRecoil.SetValue(1.f);
	WeaponSnipGroup.PlaceOtherControl("Recoil Control", this, &WeaponSnipRecoil);

	WeaponSnipAimtime.SetValue(0);
	WeaoponSnipStartAimtime.SetValue(0);

#pragma region MPs
	WeaponMpGroup.SetPosition(16 + 134, 383);
	WeaponMpGroup.SetText("MPs");
	WeaponMpGroup.SetSize(240, 136);
	RegisterControl(&WeaponMpGroup);

	WeaponMpHitbox.SetFileId("mps_hitbox");
	WeaponMpHitbox.AddItem("Head");
	WeaponMpHitbox.AddItem("Neck");
	WeaponMpHitbox.AddItem("Chest");
	WeaponMpHitbox.AddItem("Stomach");
	WeaponMpHitbox.AddItem("Multihitbox");
	WeaponMpGroup.PlaceOtherControl("Hitbox", this, &WeaponMpHitbox);

	WeaponMpSpeed.SetFileId("mps_speed");
	WeaponMpSpeed.SetBoundaries(0.f, 100.f);
	WeaponMpSpeed.SetValue(1.0f);
	WeaponMpGroup.PlaceOtherControl("Max Speed", this, &WeaponMpSpeed);

	WeaponMpFoV.SetFileId("mps_fov");
	WeaponMpFoV.SetBoundaries(0.f, 30.f);
	WeaponMpFoV.SetValue(4.f);
	WeaponMpGroup.PlaceOtherControl("Field of View", this, &WeaponMpFoV);

	WeaponMpRecoil.SetFileId("mps_recoil");
	WeaponMpRecoil.SetBoundaries(0.f, 2.f);
	WeaponMpRecoil.SetValue(1.f);
	WeaponMpGroup.PlaceOtherControl("Recoil Control", this, &WeaponMpRecoil);

	WeaponMpAimtime.SetValue(0);
	WeaoponMpStartAimtime.SetValue(0);
#pragma endregion

#pragma region Shotguns
	WeaponShotgunGroup.SetPosition(272 + 134, 375);
	WeaponShotgunGroup.SetText("Shotguns");
	WeaponShotgunGroup.SetSize(240, 136 + 8);
	RegisterControl(&WeaponShotgunGroup);

	WeaponShotgunHitbox.SetFileId("shotgun_hitbox");
	WeaponShotgunHitbox.AddItem("Head");
	WeaponShotgunHitbox.AddItem("Neck");
	WeaponShotgunHitbox.AddItem("Chest");
	WeaponShotgunHitbox.AddItem("Stomach");
	WeaponShotgunHitbox.AddItem("Multihitbox");
	WeaponShotgunGroup.PlaceOtherControl("Hitbox", this, &WeaponShotgunHitbox);

	WeaponShotgunSpeed.SetFileId("shotgun_speed");
	WeaponShotgunSpeed.SetBoundaries(0.f, 100.f);
	WeaponShotgunSpeed.SetValue(1.0f);
	WeaponShotgunGroup.PlaceOtherControl("Max Speed", this, &WeaponShotgunSpeed);

	WeaponShotgunFoV.SetFileId("shotgun_fov");
	WeaponShotgunFoV.SetBoundaries(0.f, 30.f);
	WeaponShotgunFoV.SetValue(3.f);
	WeaponShotgunGroup.PlaceOtherControl("Field of View", this, &WeaponShotgunFoV);

	WeaponShotgunRecoil.SetFileId("snip_recoil");
	WeaponShotgunRecoil.SetBoundaries(0.f, 2.f);
	WeaponShotgunRecoil.SetValue(1.f);
	WeaponShotgunGroup.PlaceOtherControl("Recoil Control", this, &WeaponShotgunRecoil);

	WeaponShotgunAimtime.SetValue(0);
	WeaoponShotgunStartAimtime.SetValue(0);

#pragma endregion

#pragma region Machineguns
	WeaponMGGroup.SetPosition(528 + 134, 375);
	WeaponMGGroup.SetText("Heavy");
	WeaponMGGroup.SetSize(240, 136 + 8);
	RegisterControl(&WeaponMGGroup);

	WeaponMGHitbox.SetFileId("mg_hitbox");
	WeaponMGHitbox.AddItem("Head");
	WeaponMGHitbox.AddItem("Neck");
	WeaponMGHitbox.AddItem("Chest");
	WeaponMGHitbox.AddItem("Stomach");
	WeaponMGHitbox.AddItem("Multihitbox");
	WeaponMGGroup.PlaceOtherControl("Hitbox", this, &WeaponMGHitbox);

	WeaponMGSpeed.SetFileId("mg_speed");
	WeaponMGSpeed.SetBoundaries(0.f, 100.f);
	WeaponMGSpeed.SetValue(1.0f);
	WeaponMGGroup.PlaceOtherControl("Max Speed", this, &WeaponMGSpeed);

	WeaponMGFoV.SetFileId("mg_fov");
	WeaponMGFoV.SetBoundaries(0.f, 30.f);
	WeaponMGFoV.SetValue(4.f);
	WeaponMGGroup.PlaceOtherControl("Field of View", this, &WeaponMGFoV);

	WeaponMGRecoil.SetFileId("mg_recoil");
	WeaponMGRecoil.SetBoundaries(0.f, 2.f);
	WeaponMGRecoil.SetValue(1.f);
	WeaponMGGroup.PlaceOtherControl("Recoil Control", this, &WeaponMGRecoil);

	WeaponMGAimtime.SetValue(0);
	WeaoponMGStartAimtime.SetValue(0);

#pragma endregion
}

void CVisualTab::Setup()
{
	SetTitle("VISUALS");

	ActiveLabel.SetPosition(46 + 134, -15 - 8);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(86 + 134, -15 - 8);
	RegisterControl(&Active);

#pragma region Options
	BoxGroup.SetText("Box");
	BoxGroup.SetPosition(363 - 25, -15 - 8);
	BoxGroup.SetSize(173, 155);
	RegisterControl(&BoxGroup);

	OptionsBox.SetFileId("otr_showbox");
	BoxGroup.PlaceCheckBox("Show Box", this, &OptionsBox);

	OptionsName.SetFileId("opt_name");
	BoxGroup.PlaceCheckBox("Name", this, &OptionsName);

	OptionsWeapon.SetFileId("opt_weapon");
	BoxGroup.PlaceCheckBox("Weapon", this, &OptionsWeapon);

	OptionHealthEnable.SetFileId("opt_health");
	BoxGroup.PlaceCheckBox("Health", this, &OptionHealthEnable);

	OptionsArmor.SetFileId("opt_armor");
	BoxGroup.PlaceCheckBox("AmmoBar", this, &OptionsArmor);

	OptionsSkeleton.SetFileId("opt_bone");
	BoxGroup.PlaceCheckBox("Skeleton", this, &OptionsSkeleton);

	ChamsGroup.SetText("Chams");
	ChamsGroup.SetPosition(16 + 134, -3);  //-23
	ChamsGroup.SetSize(173, 135);
	RegisterControl(&ChamsGroup);

	OptionsChams.SetFileId("opt_chams");
	OptionsChams.AddItem("Off");
	OptionsChams.AddItem("Normal");
	OptionsChams.AddItem("Flat");
	ChamsGroup.PlaceOtherControl("Chams", this, &OptionsChams);

	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("Off");
	OtherNoHands.AddItem("None");
	OtherNoHands.AddItem("Transparent");
	OtherNoHands.AddItem("Wireframe");
	OtherNoHands.AddItem("Chams");
	OtherNoHands.AddItem("Rainbow");
	ChamsGroup.PlaceOtherControl("Hands", this, &OtherNoHands);

	ChamsVisibleOnly.SetFileId("opt_chamsvisonly");
	ChamsGroup.PlaceCheckBox("XQZ Chams", this, &ChamsVisibleOnly);

	pLocalOpacity.SetFileId("otr_plocal_opacity");
	ChamsGroup.PlaceCheckBox("Opacity", this, &pLocalOpacity);
	
	FakeAngleChams.SetFileId("FakeAngleChams");
	FakeAngleChams.AddItem("Off");
	FakeAngleChams.AddItem("Fake");
	FakeAngleChams.AddItem("Lby");
	FakeAngleChams.AddItem("F+L");
	ChamsGroup.PlaceOtherControl("FAC", this, &FakeAngleChams);

	OptionsGroup.SetText("Misc");
	OptionsGroup.SetPosition(150, 155 + 183);
	OptionsGroup.SetSize(173, 155);
	RegisterControl(&OptionsGroup);

	OtherHitmarker.SetFileId("otr_hitmarker");
	OptionsGroup.PlaceCheckBox("Hitmarker", this, &OtherHitmarker);

	HitmarkerSound.SetFileId("otr_hitmarkersound");
	OptionsGroup.PlaceCheckBox("Hitmarker Sound", this, &HitmarkerSound);

	Logs.SetFileId("otr_logs");
	OptionsGroup.PlaceCheckBox("Logs", this, &Logs);

	OptionsAimSpot.SetFileId("opt_aimspot");
	OptionsGroup.PlaceCheckBox("Head Cross", this, &OptionsAimSpot);

	BacktrackingLol.SetFileId("opt_backdot");
	OptionsGroup.PlaceCheckBox("Backtrack Visualization", this, &BacktrackingLol);
	
	NightMode.SetFileId("opt_NightMode");
	OptionsGroup.PlaceCheckBox("Night Mode", this, &NightMode);

	InfoGroup.SetText("Info");
	InfoGroup.SetPosition(526, 155);
	InfoGroup.SetSize(173, 155);
	RegisterControl(&InfoGroup);

	OptionsInfo.SetFileId("opt_info");
	InfoGroup.PlaceCheckBox("Info", this, &OptionsInfo);

	ResolverInfo.SetFileId("opt_resolverinfo");
	InfoGroup.PlaceCheckBox("Resolver Info", this, &ResolverInfo);

	OptionsCompRank.SetFileId("opt_comprank");
	InfoGroup.PlaceCheckBox("Ranks", this, &OptionsCompRank);

	RemovalsGroup.SetText("Removals");
	RemovalsGroup.SetPosition(566 - 40, -15 - 8);
	RemovalsGroup.SetSize(173, 155);
	RegisterControl(&RemovalsGroup);


	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	RemovalsGroup.PlaceCheckBox("Remove Visual Recoil", this, &OtherNoVisualRecoil);

	OtherNoFlash.SetFileId("otr_noflash");
	RemovalsGroup.PlaceCheckBox("Remove Flash", this, &OtherNoFlash);

	OtherNoSmoke.SetFileId("otr_nosmoke");
	RemovalsGroup.PlaceCheckBox("Remove Smoke", this, &OtherNoSmoke);

	OtherNoScope.SetFileId("otr_noscope");
	RemovalsGroup.PlaceCheckBox("Remove Scope", this, &OtherNoScope);

#pragma endregion Setting up the Options controls

#pragma region Filters
	FiltersGroup.SetText("Filters");
	FiltersGroup.SetPosition(749 - 40, -15 - 8);
	FiltersGroup.SetSize(173, 155);
	RegisterControl(&FiltersGroup);

	//FiltersAll.SetFileId("ftr_all");
	//FiltersGroup.PlaceCheckBox("All", this, &FiltersAll);

	FiltersPlayers.SetFileId("ftr_players");
	FiltersGroup.PlaceCheckBox("Players", this, &FiltersPlayers);

	FiltersSelf.SetFileId("ftr_self");
	FiltersGroup.PlaceCheckBox("Self", this, &FiltersSelf);


	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	FiltersGroup.PlaceCheckBox("Enemies Only", this, &FiltersEnemiesOnly);

	FiltersWeapons.SetFileId("ftr_weaps");
	FiltersGroup.PlaceCheckBox("Weapons", this, &FiltersWeapons);

	FiltersNades.SetFileId("ftr_nades");
	FiltersGroup.PlaceCheckBox("Nades", this, &FiltersNades);

	FiltersC4.SetFileId("ftr_c4");
	FiltersGroup.PlaceCheckBox("C4", this, &FiltersC4);

#pragma endregion Setting up the Filters controls

#pragma region Other
	OtherGroup.SetText("Show");
	OtherGroup.SetPosition(150, 155);
	OtherGroup.SetSize(173, 155);
	RegisterControl(&OtherGroup);

	OtherSpreadCrosshair.SetFileId("otr_spreadhair");
	OtherGroup.PlaceCheckBox("Show Innacuracy", this, &OtherSpreadCrosshair);

	SpecList.SetFileId("otr_speclist");
	OtherGroup.PlaceCheckBox("Show Spectators", this, &SpecList);

	OtherRadar.SetFileId("otr_radar");
	OtherGroup.PlaceCheckBox("Show Radar", this, &OtherRadar);

	Watermark.SetFileId("otr_watermark");
	OtherGroup.PlaceCheckBox("Show Watermark", this, &Watermark);

	WelcomeMessage.SetFileId("otr_welcomemsg");
	WelcomeMessage.SetState(true);
	OtherGroup.PlaceCheckBox("Show Greeting", this, &WelcomeMessage);

	GlowGroup.SetText("Glow");
	GlowGroup.SetPosition(363 - 25, 155);
	GlowGroup.SetSize(173, 155);
	RegisterControl(&GlowGroup);

	OptionsGlow.SetFileId("opt_glow");
	GlowGroup.PlaceCheckBox("Glow", this, &OptionsGlow);

	EntityGlow.SetFileId("opt_entityglow");
	GlowGroup.PlaceCheckBox("Entity Glow", this, &EntityGlow);

	//GlowZ.SetFileId("opt_glowz");
	GlowZ.SetBoundaries(0.f, 255.f);
	GlowZ.SetValue(255);
	//OptionsGroup.PlaceOtherControl("Glow Alpha", this, &GlowZ);

	RenderGroup.SetText("Render");
	RenderGroup.SetPosition(709, 155);
	RenderGroup.SetSize(173, 155);
	RegisterControl(&RenderGroup);

	Barrels.SetFileId("opt_barrels");
	RenderGroup.PlaceCheckBox("Trace Lines", this, &Barrels);

	AALines.SetFileId("opt_aalines");
	RenderGroup.PlaceCheckBox("Anti Aim Lines", this, &AALines);

	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(0.f, 180.f);
	OtherViewmodelFOV.SetValue(0.f);
	RenderGroup.PlaceOtherControl("Viewmodel FOV", this, &OtherViewmodelFOV);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 70.f);
	OtherFOV.SetValue(0.f);
	RenderGroup.PlaceOtherControl("Render FOV", this, &OtherFOV);



#pragma endregion Setting up the Other controls
}

void CMiscTab::Setup()
{
	SetTitle("MISC");

#pragma region Other
	OtherGroup.SetPosition(408 + 124, -15 - 8);
	OtherGroup.SetSize(360, 270);
	OtherGroup.SetText("Other");
	RegisterControl(&OtherGroup);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceCheckBox("Anti Untrusted", this, &OtherSafeMode);

	OtherThirdperson.SetFileId("aa_thirdpsr");
	OtherGroup.PlaceCheckBox("Thirdperson", this, &OtherThirdperson);

	ThirdpersonAngle.SetFileId("aa_thirdpersonMode");
	ThirdpersonAngle.AddItem("Real");
	ThirdpersonAngle.AddItem("Fake");
	ThirdpersonAngle.AddItem("LBY");
	OtherGroup.PlaceOtherControl("Thirdperson Angle", this, &ThirdpersonAngle);

	ThirdPersonKeyBind.SetFileId("aa_thirdpersonKey");
	OtherGroup.PlaceOtherControl("Thirdperson Key", this, &ThirdPersonKeyBind);

	AutoPistol.SetFileId("otr_autopistol");
	OtherGroup.PlaceCheckBox("Auto Pistol", this, &AutoPistol);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceCheckBox("Auto Jump", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherAutoStrafe.AddItem("Off");
	OtherAutoStrafe.AddItem("Legit");
	OtherAutoStrafe.AddItem("Rage");
	OtherGroup.PlaceOtherControl("Auto Strafer", this, &OtherAutoStrafe);

	NameChanger.SetFileId("otr_spam");
	NameChanger.AddItem("Off");
	NameChanger.AddItem("Name Spam");
	NameChanger.AddItem("No Name");
	NameChanger.AddItem("Name Steal");
	NameChanger.AddItem("Crasher");
	OtherGroup.PlaceOtherControl("Name Changer", this, &NameChanger);

	OtherClantag.SetFileId("otr_clantag");
	OtherClantag.AddItem("Off");
	OtherClantag.AddItem("SchoolShooter");
	OtherClantag.AddItem("SchoolShooter2");
	OtherClantag.AddItem("Ayyware");
	OtherClantag.AddItem("no name");
	OtherClantag.AddItem("best");
	OtherClantag.AddItem("gamesense");
	OtherClantag.AddItem("BryceMemes");
	OtherClantag.AddItem("custom");
	OtherClantag.AddItem("time");
	OtherGroup.PlaceOtherControl("ClanTag", this, &OtherClantag);

	customclan.SetFileId("otr_customclan");
	customclan.SetText("SS");
	OtherGroup.PlaceOtherControl("Custom Clantag", this, &customclan);

	OtherAirStuck.SetFileId("otr_astuck");
	OtherGroup.PlaceOtherControl("Air Stuck", this, &OtherAirStuck);

#pragma endregion other random options

#pragma region FakeLag
	FakeLagGroup.SetPosition(16 + 134, -15 - 8);
	FakeLagGroup.SetSize(360, 105);
	FakeLagGroup.SetText("Fakelag");
	RegisterControl(&FakeLagGroup);

	FakeLagEnable.SetFileId("fakelag_enable");
	FakeLagGroup.PlaceCheckBox("Enable", this, &FakeLagEnable);

	FakeLagChoke.SetFileId("fakelag_choke");
	FakeLagChoke.SetBoundaries(0, 18);
	FakeLagChoke.SetValue(0);
	FakeLagGroup.PlaceOtherControl("Choke Factor", this, &FakeLagChoke);

	FakeLagTyp.SetFileId("fakelag_typ");
	FakeLagTyp.AddItem("Off");
	FakeLagTyp.AddItem("Normal");
	FakeLagTyp.AddItem("Adaptive");
	FakeLagTyp.AddItem("Normal - In Air");
	FakeLagTyp.AddItem("Adaptive - In Air");
	FakeLagTyp.AddItem("Normal - Break Lagcomp");
	FakeLagGroup.PlaceOtherControl("Mode", this, &FakeLagTyp);

#pragma endregion fakelag shit

#pragma region Buybot
	BuyBotGroup.SetPosition(16 + 134, 105 - 8);
	BuyBotGroup.SetSize(360, 150);
	BuyBotGroup.SetText("Buybot");
	RegisterControl(&BuyBotGroup);

	EnableBuyBot.SetFileId("bb_enable");
	BuyBotGroup.PlaceCheckBox("Enable", this, &EnableBuyBot);

	BuyBot.SetFileId("buybot");
	BuyBot.AddItem("Off");
	BuyBot.AddItem("Ak/M4");
	BuyBot.AddItem("AWP");
	BuyBot.AddItem("Auto + Duals");
	BuyBotGroup.PlaceOtherControl("Buy Bot", this, &BuyBot);

	BuyBotGrenades.SetFileId("buybot_grenades");
	BuyBotGrenades.AddItem("Off");
	BuyBotGrenades.AddItem("Flash + Smoke + HE");
	BuyBotGrenades.AddItem("Flash + Smoke + HE + Molo");
	BuyBotGroup.PlaceOtherControl("Buy Grenades", this, &BuyBotGrenades);

	BuyBotKevlar.SetFileId("buybot_kevlar");
	BuyBotGroup.PlaceCheckBox("Buy Armor", this, &BuyBotKevlar);

	BuyBotDefuser.SetFileId("buybot_defuser");
	BuyBotGroup.PlaceCheckBox("Buy Defuse Kit", this, &BuyBotDefuser);

	KnifeGroup.SetPosition(408 + 124, 270);
	KnifeGroup.SetText("Override Knife");
	KnifeGroup.SetSize(360, 100);
	RegisterControl(&KnifeGroup);

	SkinEnable.SetFileId("otr_skinenable");
	KnifeGroup.PlaceCheckBox("Enable", this, &SkinEnable);

	KnifeModel.SetFileId("knife_model");
	KnifeModel.AddItem("Bayonet");
	KnifeModel.AddItem("Bowie Knife");
	KnifeModel.AddItem("Butterfly Knife");
	KnifeModel.AddItem("Falchion Knife");
	KnifeModel.AddItem("Flip Knife");
	KnifeModel.AddItem("Gut Knife");
	KnifeModel.AddItem("Huntsman Knife");
	KnifeModel.AddItem("Karambit");
	KnifeModel.AddItem("M9 Bayonet");
	KnifeModel.AddItem("Shadow Daggers");
	KnifeGroup.PlaceOtherControl("Knife", this, &KnifeModel);
	
	KnifeSkin.SetFileId("knife_skin");
	KnifeGroup.PlaceOtherControl("Skin ID", this, &KnifeSkin);

	KnifeApply.SetText("Apply Changes");
	KnifeApply.SetCallback(KnifeApplyCallbk);
	KnifeApply.SetPosition(408 + 135, 400);
	KnifeApply.SetSize(360, 106);
	RegisterControl(&KnifeApply);

#pragma endregion Buybot
}

void CSettingsTab::Setup()
{
	SetTitle("SETTINGS");

	wpnhitchanceGroup.SetPosition(16 + 134, -15 - 8);
	wpnhitchanceGroup.SetSize(360, 190);
	wpnhitchanceGroup.SetText("Hitchance");
	RegisterControl(&wpnhitchanceGroup);

	WeaponCheck.SetFileId("weapon_chcek");
	wpnhitchanceGroup.PlaceCheckBox("Enable", this, &WeaponCheck);

	scoutChance.SetFileId("scout_chance");
	scoutChance.SetBoundaries(0, 100);
	scoutChance.SetValue(0);
	wpnhitchanceGroup.PlaceOtherControl("Scout", this, &scoutChance);

	AWPChance.SetFileId("awp_chance");
	AWPChance.SetBoundaries(0, 100);
	AWPChance.SetValue(0);
	wpnhitchanceGroup.PlaceOtherControl("AWP", this, &AWPChance);

	AutoChance.SetFileId("auto_chance");
	AutoChance.SetBoundaries(0, 100);
	AutoChance.SetValue(0);
	wpnhitchanceGroup.PlaceOtherControl("Auto", this, &AutoChance);

	RifleChance.SetFileId("rifle_chance");
	RifleChance.SetBoundaries(0, 100);
	RifleChance.SetValue(0);
	wpnhitchanceGroup.PlaceOtherControl("Rifle", this, &RifleChance);

	MPChance.SetFileId("mp_chance");
	MPChance.SetBoundaries(0, 100);
	MPChance.SetValue(0);
	wpnhitchanceGroup.PlaceOtherControl("MPS", this, &MPChance);

	PistolChance.SetFileId("pistol_chance");
	PistolChance.SetBoundaries(0, 100);
	PistolChance.SetValue(0);
	wpnhitchanceGroup.PlaceOtherControl("Pistols", this, &PistolChance);



	wpnmindmgGroup.SetPosition(408 + 124, -15 - 8);
	wpnmindmgGroup.SetSize(360, 190);
	wpnmindmgGroup.SetText("Minimum Damage");
	RegisterControl(&wpnmindmgGroup);

	scoutmindmg.SetFileId("scout_mindmg");
	scoutmindmg.SetBoundaries(0, 100);
	scoutmindmg.SetValue(0);
	wpnmindmgGroup.PlaceOtherControl("Scout", this, &scoutmindmg);

	AWPmindmg.SetFileId("awp_mindmg");
	AWPmindmg.SetBoundaries(0, 100);
	AWPmindmg.SetValue(0);
	wpnmindmgGroup.PlaceOtherControl("AWP", this, &AWPmindmg);

	Automindmg.SetFileId("auto_mindmg");
	Automindmg.SetBoundaries(0, 100);
	Automindmg.SetValue(0);
	wpnmindmgGroup.PlaceOtherControl("Auto", this, &Automindmg);

	Riflemindmg.SetFileId("rifle_mindmg");
	Riflemindmg.SetBoundaries(0, 100);
	Riflemindmg.SetValue(0);
	wpnmindmgGroup.PlaceOtherControl("Rifle", this, &Riflemindmg);

	MPmindmg.SetFileId("mp_mindmg");
	MPmindmg.SetBoundaries(0, 100);
	MPmindmg.SetValue(0);
	wpnmindmgGroup.PlaceOtherControl("MPS", this, &MPmindmg);

	Pistolmindmg.SetFileId("pistol_mindmg");
	Pistolmindmg.SetBoundaries(0, 100);
	Pistolmindmg.SetValue(0);
	wpnmindmgGroup.PlaceOtherControl("Pistols", this, &Pistolmindmg);

};


void CColorTab::Setup()
{
	SetTitle("COLORS");
#pragma region Visual Colors


	CTVisibleGroup.SetPosition(16 + 134, -23);
	CTVisibleGroup.SetText("CT Box Visible");
	CTVisibleGroup.SetSize(170, 110);
	RegisterControl(&CTVisibleGroup);

	CTColorVisR.SetFileId("ct_color_vis_r");
	CTColorVisR.SetBoundaries(0.f, 255.f);
	CTColorVisR.SetValue(0.f);
	CTVisibleGroup.PlaceOtherControl("Red", this, &CTColorVisR);

	CTColorVisG.SetFileId("ct_color_vis_g");
	CTColorVisG.SetBoundaries(0.f, 255.f);
	CTColorVisG.SetValue(230.f);
	CTVisibleGroup.PlaceOtherControl("Green", this, &CTColorVisG);

	CTColorVisB.SetFileId("ct_color_vis_b");
	CTColorVisB.SetBoundaries(0.f, 255.f);
	CTColorVisB.SetValue(15.f);
	CTVisibleGroup.PlaceOtherControl("Blue", this, &CTColorVisB);

	CTBoxNotVisGroup.SetPosition(272 + 134 - 70, -23);
	CTBoxNotVisGroup.SetText("CT Box Not Visible");
	CTBoxNotVisGroup.SetSize(170, 110);
	RegisterControl(&CTBoxNotVisGroup);

	CTColorNoVisR.SetFileId("ct_color_no_vis_r");
	CTColorNoVisR.SetBoundaries(0.f, 255.f);
	CTColorNoVisR.SetValue(0.f);
	CTBoxNotVisGroup.PlaceOtherControl("Red", this, &CTColorNoVisR);

	CTColorNoVisG.SetFileId("ct_color_no_vis_g");
	CTColorNoVisG.SetBoundaries(0.f, 255.f);
	CTColorNoVisG.SetValue(235.f);
	CTBoxNotVisGroup.PlaceOtherControl("Green", this, &CTColorNoVisG);

	CTColorNoVisB.SetFileId("ct_color_no_vis_b");
	CTColorNoVisB.SetBoundaries(0.f, 255.f);
	CTColorNoVisB.SetValue(10.f);
	CTBoxNotVisGroup.PlaceOtherControl("Blue", this, &CTColorNoVisB);

	TVisibleBoxGroup.SetPosition(521, -23);
	TVisibleBoxGroup.SetText("T Box Visible");
	TVisibleBoxGroup.SetSize(170, 110);
	RegisterControl(&TVisibleBoxGroup);

	TColorVisR.SetFileId("t_color_vis_r");
	TColorVisR.SetBoundaries(0.f, 255.f);
	TColorVisR.SetValue(0.f);
	TVisibleBoxGroup.PlaceOtherControl("Red", this, &TColorVisR);

	TColorVisG.SetFileId("t_color_vis_g");
	TColorVisG.SetBoundaries(0.f, 255.f);
	TColorVisG.SetValue(50.f);
	TVisibleBoxGroup.PlaceOtherControl("Green", this, &TColorVisG);

	TColorVisB.SetFileId("t_color_vis_b");
	TColorVisB.SetBoundaries(0.f, 255.f);
	TColorVisB.SetValue(220.f);
	TVisibleBoxGroup.PlaceOtherControl("Blue", this, &TColorVisB);

	Tboxnotvisiblegroup.SetPosition(521 + 184, -23);
	Tboxnotvisiblegroup.SetText("T Box Not Visible");
	Tboxnotvisiblegroup.SetSize(170, 110);
	RegisterControl(&Tboxnotvisiblegroup);

	TColorNoVisR.SetFileId("t_color_no_vis_r");
	TColorNoVisR.SetBoundaries(0.f, 255.f);
	TColorNoVisR.SetValue(0.f);
	Tboxnotvisiblegroup.PlaceOtherControl("", this, &TColorNoVisR);

	TColorNoVisG.SetFileId("t_color_no_vis_g");
	TColorNoVisG.SetBoundaries(0.f, 255.f);
	TColorNoVisG.SetValue(50.f);
	Tboxnotvisiblegroup.PlaceOtherControl("", this, &TColorNoVisG);

	TColorNoVisB.SetFileId("t_color_no_vis_b");
	TColorNoVisB.SetBoundaries(0.f, 255.f);
	TColorNoVisB.SetValue(220.f);
	Tboxnotvisiblegroup.PlaceOtherControl("", this, &TColorNoVisB);

	GlowGroup.SetPosition(521, 112);
	GlowGroup.SetText("Glow");
	GlowGroup.SetSize(170, 110);
	RegisterControl(&GlowGroup);

	GlowR.SetFileId("GlowR");
	GlowR.SetBoundaries(0.f, 255.f);
	GlowR.SetValue(230.f);
	GlowGroup.PlaceOtherControl("", this, &GlowR);

	GlowG.SetFileId("GlowG");
	GlowG.SetBoundaries(0.f, 255.f);
	GlowG.SetValue(0.f);
	GlowGroup.PlaceOtherControl("", this, &GlowG);

	GlowB.SetFileId("GlowB");
	GlowB.SetBoundaries(0.f, 255.f);
	GlowB.SetValue(0.f);
	GlowGroup.PlaceOtherControl("", this, &GlowB);

	CTCHamsGroup.SetPosition(16 + 134, 112);
	CTCHamsGroup.SetText("CT Chams");
	CTCHamsGroup.SetSize(170, 110);
	RegisterControl(&CTCHamsGroup);

	CTChamsR.SetFileId("ctchamsr");
	CTChamsR.SetBoundaries(0.f, 255.f);
	CTChamsR.SetValue(40.f);
	CTCHamsGroup.PlaceOtherControl("", this, &CTChamsR);

	CTChamsG.SetFileId("ctchamsg");
	CTChamsG.SetBoundaries(0.f, 255.f);
	CTChamsG.SetValue(120.f);
	CTCHamsGroup.PlaceOtherControl("", this, &CTChamsG);

	CTChamsB.SetFileId("ctchamsb");
	CTChamsB.SetBoundaries(0.f, 255.f);
	CTChamsB.SetValue(170.f);
	CTCHamsGroup.PlaceOtherControl("", this, &CTChamsB);

	TChamsGroup.SetPosition(272 + 64, 112);
	TChamsGroup.SetText("T Chams");
	TChamsGroup.SetSize(170, 110);
	RegisterControl(&TChamsGroup);

	TChamsR.SetFileId("tchamsr");
	TChamsR.SetBoundaries(0.f, 255.f);
	TChamsR.SetValue(210.f);
	TChamsGroup.PlaceOtherControl("", this, &TChamsR);

	TChamsG.SetFileId("tchamsg");
	TChamsG.SetBoundaries(0.f, 255.f);
	TChamsG.SetValue(20.f);
	TChamsGroup.PlaceOtherControl("", this, &TChamsG);

	TChamsB.SetFileId("tchamsb");
	TChamsB.SetBoundaries(0.f, 255.f);
	TChamsB.SetValue(70.f);
	TChamsGroup.PlaceOtherControl("", this, &TChamsB);

	MenuColorGroup.SetPosition(521 + 184, 112);
	MenuColorGroup.SetText("Menu");
	MenuColorGroup.SetSize(170, 110);
	RegisterControl(&MenuColorGroup);

	ColorMenuAccentR.SetFileId("menuaccr");
	ColorMenuAccentR.SetBoundaries(0, 255);
	ColorMenuAccentR.SetValue(16);
	MenuColorGroup.PlaceOtherControl("", this, &ColorMenuAccentR);

	ColorMenuAccentG.SetFileId("menuaccg");
	ColorMenuAccentG.SetBoundaries(0, 255);
	ColorMenuAccentG.SetValue(123);
	MenuColorGroup.PlaceOtherControl("", this, &ColorMenuAccentG);

	ColorMenuAccentB.SetFileId("menuaccb");
	ColorMenuAccentB.SetBoundaries(0, 255);
	ColorMenuAccentB.SetValue(255);
	MenuColorGroup.PlaceOtherControl("", this, &ColorMenuAccentB);

#pragma endregion
}

void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoUIFrame()
{
	if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 3)
	{
		Window.RageBotTab.AdvancedResolver.SetState(false);
	}

	GUI.Update();
	GUI.Draw();
	
}