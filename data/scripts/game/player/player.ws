/***********************************************************************/
/** 	© 2015 CD PROJEKT S.A. All rights reserved.
/** 	THE WITCHER® is a trademark of CD PROJEKT S. A.
/** 	The Witcher game is based on the prose of Andrzej Sapkowski.
/***********************************************************************/



enum EPlayerDeathType
{
	PDT_Normal		= 0,
	PDT_Fall		= 1,
	PDT_KnockBack	= 2,
}

statemachine import abstract class CPlayer extends CActor
{		
	
	private var _DEBUGDisplayRadiusMinimapIcons : bool;
	private var debug_BIsInputAllowedLocks : array<name>;			
	
	
	
	
	public const var ENTER_SWIMMING_WATER_LEVEL : float;
	default ENTER_SWIMMING_WATER_LEVEL = -1.45;
	
	
	public var useSprintingCameraAnim	: bool;		default	useSprintingCameraAnim	= false;
	public var oTCameraOffset 			: float;	default oTCameraOffset 			= 0.f;	
	public var oTCameraPitchOffset 		: float;	default oTCameraPitchOffset		= 0.f;
	

	
	public				var bIsRollAllowed				: bool;
	protected			var bIsInCombatAction			: bool;
	protected			var bIsInCombatActionFriendly	: bool;
	private				var bIsInputAllowed				: bool;
	public 				var bIsFirstAttackInCombo		: bool;
	protected			var bIsInHitAnim				: bool;
	import				var enemyUpscaling				: bool;
	
	public editable		var FarZoneDistMax				: float;
	public editable 	var DangerZoneDistMax			: float;
			
		default	FarZoneDistMax = 30;
		default	DangerZoneDistMax = 50;
		
	
	private 			var commentaryCooldown			: float;
	private				var commentaryLastTime			: EngineTime;
	
	private				var	canPlaySpecificVoiceset		: bool;
		default canPlaySpecificVoiceset = true;
 	
	
	public				var isHorseMounted				: bool; 
	public				var isCompanionFollowing		: bool; 
	public				var bStartScreenIsOpened		: bool; 
	public				var bEndScreenIsOpened			: bool; 
	public				var fStartScreenFadeDuration	: float;
	public				var bStartScreenEndWithBlackScreen : bool; 
	public				var fStartScreenFadeInDuration	: float;
	const 				var DEATH_SCREEN_OPEN_DELAY		: float; 
		
		default bStartScreenIsOpened = false;	
		default bEndScreenIsOpened = false;	
		default DEATH_SCREEN_OPEN_DELAY = 2.6f; 
		default fStartScreenFadeDuration = 3.0; 
		default fStartScreenFadeInDuration = 3.0; 
		default bStartScreenEndWithBlackScreen = false; 

	
	public 				var bLAxisReleased				: bool;
	public 				var bRAxisReleased				: bool;
	private 			var bUITakesInput				: bool; 
	protected	saved	var inputHandler 				: CPlayerInput;
	public				var sprintActionPressed			: bool;
	private				var inputModuleNeededToRun		: float;
	
		default bUITakesInput = false;
		default bLAxisReleased = true;
		default inputModuleNeededToRun = -10.0; 
		
	
	private				var bInteractionPressed			: bool;	
		
	
	public				var rawPlayerSpeed 				: float; 
	public	 			var rawPlayerAngle		 		: float; 
	public	 			var rawPlayerHeading			: float; 
	
	public				var cachedRawPlayerHeading		: float;
	public				var cachedCombatActionHeading 			: float;
	public				var canResetCachedCombatActionHeading 	: bool;	
	protected			var combatActionHeading			: float;
	public				var rawCameraHeading			: float; 
	private	 			var isSprinting 				: bool;
	protected			var	isRunning					: bool;
	protected			var	isWalking					: bool;
	public	 			var playerMoveType				: EPlayerMoveType;
	private				var sprintingTime				: float;
	private 			var walkToggle 					: bool;		default walkToggle = false;
	private 			var sprintToggle 				: bool;		default sprintToggle = false;
	import public 		var isMovable 					: bool;

	public				var moveTarget					: CActor;
	public				var nonActorTarget				: CGameplayEntity;
	public				var tempLookAtTarget			: CGameplayEntity;
	public				var lockTargetSelectionHeading	: float;
		
	protected 			var rawLeftJoyRot 				: float;	
	protected 			var rawRightJoyRot  			: float;
	protected			var rawLeftJoyVec 				: Vector;
	protected			var rawRightJoyVec 				: Vector;
	protected			var prevRawLeftJoyVec 			: Vector;
	protected			var prevRawRightJoyVec 			: Vector;
	protected			var lastValidLeftJoyVec 		: Vector;
	protected			var lastValidRightJoyVec 		: Vector;
	
	public				var allowStopRunCheck			: bool;
	public				var moveTargetDampValue			: float;
	
	
	
	public 				var interiorCamera 				: bool;
	public 				var movementLockType			: EPlayerMovementLockType;
	public	 			var scriptedCombatCamera 		: bool;
	public				var modifyPlayerSpeed			: bool;
	public saved		var autoCameraCenterToggle 		: bool;
	
	default interiorCamera = false;
	default scriptedCombatCamera =  true;
		
	
	public				var inv 						: CInventoryComponent;
	
	public				saved var equipmentSlotHistory			: array<SItemUniqueId>;
	
	
	private var currentTrackedQuestSystemObjectives : array<SJournalQuestObjectiveData>;
	private var currentTrackedQuestObjectives : array<SJournalQuestObjectiveData>;
	private var currentTrackedQuestGUID : CGUID;
	private var HAXNewObjTable : array<CGUID>;
	
	
	public				var handAimPitch				: float;
	private saved		var vehicleCachedSign			: ESignType;
	
	default vehicleCachedSign = ST_None;
	
	
	public editable		var softLockDist				: float;
	public editable		var softLockFrameSize			: float;			
	public	 			var findMoveTargetDist			: float;
	public				var softLockDistVehicle			: float;
	private				var bBIsLockedToTarget			: bool;				
	private				var bActorIsLockedToTarget		: bool;				
	private				var bIsHardLockedTotarget		: bool;				
	
		default softLockDist =  12.f;
		default softLockFrameSize = 1.25f;
		default softLockDistVehicle = 30.f;
		
	
	private var terrTypeOne : ETerrainType;
	private var terrTypeTwo : ETerrainType;
	private var terrModifier : float;			
	private var prevTerrType : ETerrainType;
	
		default terrTypeOne = TT_Normal;
		default terrTypeTwo = TT_Normal;
		default terrModifier = 0.f;
		default prevTerrType = TT_Normal;
		
	
	protected var currentlyUsedItem 			 : W3UsableItem;
	protected var currentlyEquipedItem 			 : SItemUniqueId;
	protected var currentlyUsedItemL 		     : W3UsableItem;
	public saved   var currentlyEquipedItemL 		 : SItemUniqueId;
	protected var isUsableItemBlocked   		 : bool;
	protected var isUsableItemLtransitionAllowed : bool;
	protected var playerActionToRestore			 : EPlayerActionToRestore; default playerActionToRestore =  PATR_Default;
	
	public saved var teleportedOnBoatToOtherHUB : bool;
	default teleportedOnBoatToOtherHUB = false;
	
	
	protected var photomodeManager : PhotomodeManager;

	
	
	public var isAdaptiveBalance : bool;
	default isAdaptiveBalance = false;

	function IsAdaptiveBalance() : bool
	{
		return isAdaptiveBalance;
	}
	function SetAdaptiveBalance( val : bool )
	{
		Log("Adaptive balance: " + val );
		isAdaptiveBalance = val;
	}
	
	public function SetTeleportedOnBoatToOtherHUB( val : bool )
	{
		teleportedOnBoatToOtherHUB = val;
	}
	
	
	
	
	
	import final function LockButtonInteractions( channel : int );
	
	
	import final function UnlockButtonInteractions( channel : int );

	import final function GetActiveExplorationEntity() : CEntity;
	
	
			
	event OnSpawned( spawnData : SEntitySpawnData )
	{	
		var conf : int;
		
		inv = GetInventory();
		
		super.OnSpawned( spawnData );
		
		RegisterCollisionEventsListener();		
		AddTimer( 'PlayerTick', 0.f, true );
		InitializeParryType();
		SetCanPlayHitAnim( true );

		
		theGame.RemoveTimeScale( theGame.GetTimescaleSource(ETS_DebugInput) );
		
		
		if( inv )
		{
			inv.Created();	
		}
		if( !spawnData.restored )
		{
			inputHandler = new CPlayerInput in this;
			theGame.EnableUberMovement( true );
			((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).SetVarValue( 'Gameplay', 'EnableUberMovement', 1 );
		}
		
		inputHandler.Initialize(spawnData.restored );
		SetAutoCameraCenter( ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Gameplay', 'AutoCameraCenter' ) );

		SetEnemyUpscaling( ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Gameplay', 'EnemyUpscaling' ) );
		
		if( !IsNameValid(GetCurrentStateName()) )
		{
			GotoStateAuto();
		}
		
		isRunning	= false;
		SetIsWalking( false );
		
		
		EnableBroadcastPresence(true);
		
		
		AddAnimEventCallback('EquipItem',			'OnAnimEvent_QuickSlotItems');
		AddAnimEventCallback('UseItem',				'OnAnimEvent_QuickSlotItems');
		AddAnimEventCallback('HideItem',			'OnAnimEvent_QuickSlotItems');
		AddAnimEventCallback('EquipItemL',			'OnAnimEvent_QuickSlotItems');
		AddAnimEventCallback('UseItemL',			'OnAnimEvent_QuickSlotItems');
		AddAnimEventCallback('HideItemL',			'OnAnimEvent_QuickSlotItems');
		AddAnimEventCallback('AllowInput',			'OnAnimEvent_AllowInput');
		AddAnimEventCallback('DisallowInput',		'OnAnimEvent_DisallowInput');
		AddAnimEventCallback('DisallowHitAnim',		'OnAnimEvent_DisallowHitAnim');
		AddAnimEventCallback('AllowHitAnim',		'OnAnimEvent_AllowHitAnim');
		
		AddAnimEventCallback('SetRagdoll',			'OnAnimEvent_SetRagdoll');
		AddAnimEventCallback('InAirKDCheck',		'OnAnimEvent_InAirKDCheck');
		AddAnimEventCallback('EquipMedallion',		'OnAnimEvent_EquipMedallion');
		AddAnimEventCallback('HideMedallion',		'OnAnimEvent_HideMedallion');
		
		
		ResumeStaminaRegen( 'Sprint' );
		
		photomodeManager = new PhotomodeManager in this;
		photomodeManager.Initialize();
		
		
		
		invertedLockOption = ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Controls', 'InvertLockOption' );
		invertedControllerCameraX = ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Controls', 'InvertCameraX' );
		invertedControllerCameraY = ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Controls', 'InvertCameraY' );
		invertedMouseCameraX = ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Controls', 'InvertCameraXOnMouse' );
		invertedMouseCameraY = ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue( 'Controls', 'InvertCameraYOnMouse' );
		
	}
	
	
	public function Debug_ResetInput()
	{
		inputHandler = new CPlayerInput in this;
		inputHandler.Initialize(false);
	}
	
	
	
	public function GetTutorialInputHandler() : W3PlayerTutorialInput
	{
		return (W3PlayerTutorialInput)inputHandler;
	}
	
	public function BlockAction( action : EInputActionBlock, sourceName : name, optional keepOnSpawn : bool, optional isFromQuest : bool, optional isFromPlace : bool ) : bool
	{
		if ( inputHandler )
		{
			inputHandler.BlockAction(action, sourceName, true, keepOnSpawn, this, isFromQuest, isFromPlace);
			return true;
		}		
		return false;
	}
	
	public function UnblockAction( action : EInputActionBlock, sourceName : name) : bool
	{
		if ( inputHandler )
		{
			inputHandler.BlockAction(action, sourceName, false);
			return true;
		}		
		return false;
	}
	
	public final function TutorialForceUnblockRadial() : array<SInputActionLock>
	{
		var null : array<SInputActionLock>;
		
		if ( inputHandler )
		{
			return inputHandler.TutorialForceUnblockRadial();
		}
		
		return null;
	}
	
	public final function TutorialForceRestoreRadialLocks(radialLocks : array<SInputActionLock>)
	{
		if ( inputHandler )
		{
			inputHandler.TutorialForceRestoreRadialLocks(radialLocks);
		}
	}
	
	public function GetActionLocks( action : EInputActionBlock ) : array< SInputActionLock >
	{
		return inputHandler.GetActionLocks( action );
	}
	
	public function GetAllActionLocks() : array< array< SInputActionLock > >
	{
		return inputHandler.GetAllActionLocks();
	}
	
	public function IsActionAllowed( action : EInputActionBlock ) : bool
	{
		if ( inputHandler )
		{
			return inputHandler.IsActionAllowed( action );
		}
		return true;
	}
	
	public function IsActionBlockedBy( action : EInputActionBlock, sourceName : name ) : bool
	{
		if ( inputHandler )
		{
			return inputHandler.IsActionBlockedBy(action,sourceName);
		}
		return false;	
	}
	
	public function IsWeaponActionAllowed( weapon : EPlayerWeapon ) : bool
	{
		if ( inputHandler )
		{
			
			
			if ( weapon == PW_Fists )
			{
				return inputHandler.IsActionAllowed( EIAB_Fists );
			}
			else
			{
				return inputHandler.IsActionAllowed( EIAB_DrawWeapon );
			}
		}
		return true;
	}
	
	public function BlockAllActions(sourceName : name, lock : bool, optional exceptions : array<EInputActionBlock>, optional exceptUI : bool, optional saveLock : bool, optional onSpawnedNullPointerHackFix : CPlayer, optional isFromPlace : bool)
	{
		if(inputHandler)
		{
			if(exceptUI)
			{
				exceptions.PushBack(EIAB_OpenInventory);
				exceptions.PushBack(EIAB_MeditationWaiting);
				exceptions.PushBack(EIAB_FastTravel);
				exceptions.PushBack(EIAB_OpenMap);
				exceptions.PushBack(EIAB_OpenCharacterPanel);
				exceptions.PushBack(EIAB_OpenJournal);
				exceptions.PushBack(EIAB_OpenAlchemy);
			}
			inputHandler.BlockAllActions(sourceName, lock, exceptions, saveLock, onSpawnedNullPointerHackFix, false, isFromPlace);
		}
		if(lock)
		{
			
		}
	}
	
	public final function BlockAllQuestActions(sourceName : name, lock : bool)
	{
		inputHandler.BlockAllQuestActions(sourceName, lock);
	}
	
	public function BlockAllUIQuestActions(sourceName : name, lock : bool)
	{
		inputHandler.BlockAllUIQuestActions(sourceName, lock);
	}
			
	
	
	public function GetInputHandler() : CPlayerInput
	{
		return inputHandler;
	}

		
	public function CheatGod2(on : bool)
	{
		if(on)
			SetImmortalityMode( AIM_Immortal, AIC_Default, true );
		else
			SetImmortalityMode( AIM_None, AIC_Default, true );	
		
		StaminaBoyInternal(on);
	}
	
	public function IsInCombatState() : bool
	{
		var stateName : name;
		stateName = thePlayer.GetCurrentStateName();
		if ( stateName == 'Combat' || stateName == 'CombatSteel' || stateName == 'CombatSilver' || stateName == 'CombatFists' )
		{
			return true;
		}
		return false;	
	}
	
	public function DisableCombatState()
	{
		if ( IsInCombatState() )
		{
			GotoState( 'Exploration' );
		}
	}
	
	protected function SetAbilityManager()
	{
		abilityManager = new W3PlayerAbilityManager in this;	
	}
	
	
	event OnDamageFromBoids( damage : float )
	{		
		var damageAction : W3DamageAction = new W3DamageAction in theGame.damageMgr;
		
		damageAction.Initialize(NULL,this,NULL,'boid',EHRT_None,CPS_AttackPower,false,false,false,true);
		damageAction.AddDamage(theGame.params.DAMAGE_NAME_PHYSICAL,6.f);		
		damageAction.SetHitAnimationPlayType(EAHA_ForceNo);
		
		
		
		damageAction.SetSuppressHitSounds(true); 
		
		Log( "DAMAGE FROM BOID!!!!! " + damage );
		
		
		theGame.damageMgr.ProcessAction( damageAction );
		
		delete damageAction;
	}
	
	
	function InitializeParryType()
	{
		var i, j : int;
		
		parryTypeTable.Resize( EnumGetMax('EAttackSwingType')+1 );
		for( i = 0; i < EnumGetMax('EAttackSwingType')+1; i += 1 )
		{
			parryTypeTable[i].Resize( EnumGetMax('EAttackSwingDirection')+1 );
		}
		parryTypeTable[AST_Horizontal][ASD_UpDown] = PT_None;
		parryTypeTable[AST_Horizontal][ASD_DownUp] = PT_None;
		parryTypeTable[AST_Horizontal][ASD_LeftRight] = PT_None;
		parryTypeTable[AST_Horizontal][ASD_RightLeft] = PT_None;
		parryTypeTable[AST_Vertical][ASD_UpDown] = PT_None;
		parryTypeTable[AST_Vertical][ASD_DownUp] = PT_None;
		parryTypeTable[AST_Vertical][ASD_LeftRight] = PT_None;
		parryTypeTable[AST_Vertical][ASD_RightLeft] = PT_None;
		parryTypeTable[AST_DiagonalUp][ASD_UpDown] = PT_None;
		parryTypeTable[AST_DiagonalUp][ASD_DownUp] = PT_None;
		parryTypeTable[AST_DiagonalUp][ASD_LeftRight] = PT_None;
		parryTypeTable[AST_DiagonalUp][ASD_RightLeft] = PT_None;
		parryTypeTable[AST_DiagonalDown][ASD_UpDown] = PT_None;
		parryTypeTable[AST_DiagonalDown][ASD_DownUp] = PT_None;
		parryTypeTable[AST_DiagonalDown][ASD_LeftRight] = PT_None;
		parryTypeTable[AST_DiagonalDown][ASD_RightLeft] = PT_None;
		parryTypeTable[AST_Jab][ASD_UpDown] = PT_None;
		parryTypeTable[AST_Jab][ASD_DownUp] = PT_None;
		parryTypeTable[AST_Jab][ASD_LeftRight] = PT_None;
		parryTypeTable[AST_Jab][ASD_RightLeft] = PT_None;	
	}
	
	event OnPlayerTickTimer( deltaTime : float )
	{	
	}
	
	timer function PlayerTick( deltaTime : float , id : int)
	{	
		deltaTime = theTimer.timeDeltaUnscaled;
		OnPlayerTickTimer( deltaTime );
		
		if( !IsActionAllowed( EIAB_RunAndSprint ) )
		{
			movementLockType	= PMLT_NoRun;
		}
		else if( !IsActionAllowed( EIAB_Sprint ) )
		{
			movementLockType	= PMLT_NoSprint;
		}
		else
		{
			movementLockType	= PMLT_Free;
		}	
	}
	
	
	function IsLookInputIgnored() : bool
	{
		return false;
	}

	private var inputHeadingReady : bool;
	public function SetInputHeadingReady( flag : bool )
	{
		inputHeadingReady =  flag;
	}
	
	public function IsInputHeadingReady() : bool
	{
		return inputHeadingReady;
	}	

	var lastAxisInputIsMovement : bool;
	function HandleMovement( deltaTime : float )
	{
		var leftStickVector 	: Vector;
		var rightStickVector 	: Vector; 
	
		var rawLengthL 	: float;
		var rawLengthR	: float;
	
		var len : float;
		var i : int;	
	
		prevRawLeftJoyVec = rawLeftJoyVec;
		prevRawRightJoyVec = rawRightJoyVec;

		rawLeftJoyVec.X = theInput.GetActionValue( 'GI_AxisLeftX' ); 
		rawLeftJoyVec.Y = theInput.GetActionValue( 'GI_AxisLeftY' );

		if ( thePlayer.IsPCModeEnabled() )
		{
			rawRightJoyVec.X = theInput.GetActionValue( 'GI_MouseDampX' ); 
			rawRightJoyVec.Y = theInput.GetActionValue( 'GI_MouseDampY' ); 			
		}
		else
		{
			rawRightJoyVec.X = theInput.GetActionValue( 'GI_AxisRightX' ); 
			rawRightJoyVec.Y = theInput.GetActionValue( 'GI_AxisRightY' ); 
		}
			
		leftStickVector = rawLeftJoyVec;
		rightStickVector = rawRightJoyVec;
		
		
		if ( VecDot2D( prevRawLeftJoyVec, leftStickVector ) < 0.0f )
		{
			leftStickVector = lastValidLeftJoyVec;
		}
		else
		{
			lastValidLeftJoyVec = leftStickVector;
		}
		if ( VecDot2D( prevRawRightJoyVec, rightStickVector ) < 0.0f )
		{
			rightStickVector = lastValidRightJoyVec;
		}
		else
		{
			lastValidRightJoyVec = rightStickVector;
		}
		
		rawLengthL = VecLength( leftStickVector );
		rawLengthR = VecLength( rightStickVector );
		SetBehaviorVariable( 'lAxisLength', ClampF( rawLengthL, 0.0f, 1.0f ) );
		
		
		rawLeftJoyRot = VecHeading( leftStickVector );
		rawRightJoyRot = VecHeading( rightStickVector );
		
		if( rawLengthL > 0 )
		{
			bLAxisReleased = false;
			if( isSprinting )
			{
				if ( rawLengthL > 0.6 )
				{
					rawPlayerSpeed = 1.3;
					allowStopRunCheck = true;
					RemoveTimer( 'StopRunDelayedInputCheck' );
				}
				else
				{
					if ( allowStopRunCheck )
					{
						allowStopRunCheck = false;
						AddTimer( 'StopRunDelayedInputCheck', 0.25f, false );
					}
				}
			}
			else
			{
				if ( this.GetCurrentStateName() == 'Exploration' )
				{
					rawPlayerSpeed = 0.9*rawLengthL;
				}
				else
				{
					if ( rawLengthL > 0.6 )
					{
						rawPlayerSpeed = 0.8;
					}
					else
					{
						rawPlayerSpeed = 0.4;
					}
				}
			}
		}
		else
		{
			if ( isSprinting )
			{
				if  ( allowStopRunCheck )
				{
					allowStopRunCheck = false;
					AddTimer( 'StopRunDelayedInputCheck', 0.25f, false );
				}
			}
			else
			{	
				rawPlayerSpeed = 0.f;
			}
			bLAxisReleased = true;
		}
		
		if ( rawLengthR > 0 )
			bRAxisReleased = false;
		else
			bRAxisReleased = true;
		
		ProcessLAxisCaching();
		
		SetBehaviorVariable( 'moveSpeedWhileCasting', rawPlayerSpeed );

		if ( rawPlayerSpeed > 0.f )
		{
			rawPlayerHeading = AngleDistance( theCamera.GetCameraHeading(), -rawLeftJoyRot );
			if ( rawPlayerSpeed > 0.1f )
			{
				cachedRawPlayerHeading = rawPlayerHeading; 
				
			}
			if ( IsInCombatAction() )
			{
				canResetCachedCombatActionHeading = false;
				cachedCombatActionHeading = cachedRawPlayerHeading;
			}
		}

		
		rawPlayerAngle = AngleDistance( rawPlayerHeading, GetHeading() );

		if ( !ProcessLockTargetSelectionInput( rightStickVector, rawLengthR ) )
			ProcessLockTargetSelectionInput( rightStickVector, rawLengthR );
	}

	protected function ProcessLAxisCaching()
	{
		if ( bLAxisReleased )
		{
			if ( GetBIsCombatActionAllowed() )
			{
				if ( !lAxisReleaseCounterEnabled )
				{
					lAxisReleaseCounterEnabled = true;
					AddTimer( 'LAxisReleaseCounter', 0.25f );
				}
			}
			
			if ( !lAxisReleaseCounterEnabledNoCA  )
			{
				lAxisReleaseCounterEnabledNoCA  = true;
				AddTimer( 'LAxisReleaseCounterNoCA', 0.2f );
			}

			if ( !bRAxisReleased )
			{
				if ( thePlayer.IsPCModeEnabled() )
				{
					if ( lAxisReleasedAfterCounter )
						lastAxisInputIsMovement = false;
				}
				else
					lastAxisInputIsMovement = false;
			}
		}
		else
		{
			lAxisReleasedAfterCounter = false;
			lAxisReleasedAfterCounterNoCA = false;
			RemoveTimer( 'LAxisReleaseCounter' );
			RemoveTimer( 'LAxisReleaseCounterNoCA' );
			lAxisReleaseCounterEnabled = false;
			lAxisReleaseCounterEnabledNoCA = false;
			
			lastAxisInputIsMovement = true;
		}		
	}

	public function ResetLastAxisInputIsMovement()
	{
		lastAxisInputIsMovement = true;
	}
	
	
	private var invertedLockOption : bool;
	private var invertedControllerCameraX, invertedControllerCameraY : bool;
	private var invertedMouseCameraX, invertedMouseCameraY : bool;
	
	public function SetInvertedLockOption(set : bool) {invertedLockOption = set;}
	public function SetInvertedCameraX(set : bool) {invertedControllerCameraX = set;}
	public function SetInvertedCameraY(set : bool) {invertedControllerCameraY = set;}
	public function SetInvertedMouseCameraX(set : bool) {invertedMouseCameraX = set;}
	public function SetInvertedMouseCameraY(set : bool) {invertedMouseCameraY = set;}
	

	private var bRAxisReleasedLastFrame 	: bool;
	private var selectTargetTime 			: float;
	
	private var swipeMouseTimeStamp : float;
	private var swipeMouseDir 		: Vector;
	private var swipeMouseDist		: float;
	private var enableSwipeCheck  	: bool;
	protected function ProcessLockTargetSelectionInput( rightStickVector : Vector, rawLengthR : float ) : bool
	{
		var currTime	: float;
		var rightStickVectorNormalized : Vector;
		var dot	: float;
		
		if ( this.IsCameraLockedToTarget() )
		{
			currTime = theGame.GetEngineTimeAsSeconds();	

			
			if(invertedLockOption)
			{
				if(thePlayer.IsPCModeEnabled())
				{
					if(invertedMouseCameraX) rightStickVector.X *= -1;
					if(invertedMouseCameraY) rightStickVector.Y *= -1;
				}
				else
				{
					if(invertedControllerCameraX) rightStickVector.X *= -1;
					if(invertedControllerCameraY) rightStickVector.Y *= -1;
				}
			}
			
			
			if ( thePlayer.IsPCModeEnabled() )
			{
				if ( rawLengthR > 0.f )
				{
					rightStickVectorNormalized = VecNormalize( rightStickVector );
				
					if ( enableSwipeCheck )
					{
						enableSwipeCheck = false;
						swipeMouseTimeStamp = currTime;
						swipeMouseDir = rightStickVector;
						swipeMouseDist = 0.f;
					}

					dot = VecDot( swipeMouseDir, rightStickVector );
					
					if ( dot > 0.8 )
					{
						swipeMouseDist += rawLengthR;
					}
					else
					{
						enableSwipeCheck = true;
						return false;
					}
					
					swipeMouseDir = rightStickVector;					
											

					if ( currTime > swipeMouseTimeStamp + 0.2f )
					{
							
						swipeMouseDist = 0.f;
						enableSwipeCheck = true;
					}

				}
				else
				{
						
					swipeMouseDist = 0.f;
					enableSwipeCheck = true;
				}
				
				if ( swipeMouseDist <= 350.f )
					return true;
				else
				{
					rightStickVector = rightStickVectorNormalized;
					rawLengthR = VecLength( rightStickVector );
				}
			}
			
			if ( bRAxisReleasedLastFrame )
			{
				if ( rawLengthR >= 0.3 )
				{
					inputHandler.OnCbtSelectLockTarget( rightStickVector );
					selectTargetTime = currTime;
				}
			}
			else if ( rawLengthR >= 0.3 && currTime > ( selectTargetTime + 0.5f ) )
			{
				inputHandler.OnCbtSelectLockTarget( rightStickVector );
				selectTargetTime = currTime;	
			}
		}

		if ( rawLengthR < 0.3 )
			bRAxisReleasedLastFrame = true;
		else
			bRAxisReleasedLastFrame = false;
			
		return true;	
	}
	
	public var lAxisReleasedAfterCounter 	: bool;
	public var lAxisReleaseCounterEnabled 	: bool;
	private timer function LAxisReleaseCounter( time : float , id : int)
	{
		
		if ( bLAxisReleased )
			lAxisReleasedAfterCounter = true;
	}
	
	public var lAxisReleasedAfterCounterNoCA 	: bool; 
	public var lAxisReleaseCounterEnabledNoCA 	: bool;
	private timer function LAxisReleaseCounterNoCA( time : float , id : int)
	{
		
		if ( bLAxisReleased )
			lAxisReleasedAfterCounterNoCA = true;
	}
	
	
	timer function StopRunDelayedInputCheck( time : float, id : int)
	{
		
		allowStopRunCheck = true;
	}	
	
	public function IsUITakeInput() : bool 
	{
		return bUITakesInput;
	}

	public function SetUITakeInput ( val : bool ) 
	{
		bUITakesInput = val;
	}
	
	public function GetRawLeftJoyRot() : float
	{
		return rawLeftJoyRot;
	}
	
	public function SetIsActorLockedToTarget( flag : bool )
	{
		bActorIsLockedToTarget = flag;
	}
	
	public function IsActorLockedToTarget() : bool
	{
		return bActorIsLockedToTarget;
	}	
	
	
	public function SetIsCameraLockedToTarget( flag : bool )
	{
		bBIsLockedToTarget = flag;
	}
	
	public function IsCameraLockedToTarget() : bool
	{
		return bBIsLockedToTarget;
	}	
	
	public function IsLockedToTarget() : bool
	{
		return false;
	}
	
	public function EnableHardLock( flag : bool )
	{
		if ( !flag )
			Log( "EnableHardLock : false" );
		
		bIsHardLockedTotarget = flag;
	}
	
	public function IsHardLockEnabled() : bool
	{
		return bIsHardLockedTotarget;
	}	
	
	
	
	
	function EnableBroadcastPresence( enable : bool )
	{
		if ( enable )
		{
			theGame.GetBehTreeReactionManager().CreateReactionEventIfPossible( this, 'PlayerPresenceAction', -1.f , 10.0f, 3.f, -1, true); 
			theGame.GetBehTreeReactionManager().CreateReactionEventIfPossible( this, 'PlayerPresenceActionFar', -1.f , 20.0f, 3.f, -1, true); 
		}
		else
		{
			theGame.GetBehTreeReactionManager().RemoveReactionEvent(this, 'PlayerPresenceAction');
			theGame.GetBehTreeReactionManager().RemoveReactionEvent(this, 'PlayerPresenceActionFar');
		}
	}
	
	function RemoveReactions()
	{
		theGame.GetBehTreeReactionManager().RemoveReactionEvent( this, 'DrawSwordAction' );
		theGame.GetBehTreeReactionManager().RemoveReactionEvent( this, 'CombatNearbyAction' );
		theGame.GetBehTreeReactionManager().RemoveReactionEvent( this, 'AttackAction' );
		theGame.GetBehTreeReactionManager().RemoveReactionEvent( this, 'CastSignAction' );
		EnableBroadcastPresence(false);
	}
	
	function RestartReactionsIfNeeded()
	{
		EnableBroadcastPresence(true);
		
		
		
	}
	
	
	
	
	event OnBlockingSceneStarted( scene: CStoryScene )
	{
		super.OnBlockingSceneStarted( scene );
		ClearAttitudes( true, false, false );
		RaiseForceEvent( 'ForceIdle' );
		RemoveReactions();
		theGame.GetBehTreeReactionManager().CreateReactionEventIfPossible( thePlayer, 'PlayerInScene', -1.f, 60.0f, -1, -1, true ); 
		PushState( 'PlayerDialogScene' );		
	}

	event OnBlockingSceneStarted_OnIntroCutscene( scene: CStoryScene )
	{
		super.OnBlockingSceneStarted_OnIntroCutscene( scene );
	}
	
	event OnBlockingSceneEnded( optional output : CStorySceneOutput)
	{
		var exceptions : array<EInputActionBlock>;
		
		super.OnBlockingSceneEnded( output );
		
		RestartReactionsIfNeeded();
		
		exceptions.PushBack(EIAB_Movement);
		exceptions.PushBack(EIAB_RunAndSprint);
		exceptions.PushBack(EIAB_Sprint);
		this.BlockAllActions('SceneEnded',true,exceptions);
		
		this.AddTimer('RemoveSceneEndedActionBlock',1.f,false);
	}
	
	private timer function RemoveSceneEndedActionBlock( dt : float , id : int)
	{
		this.BlockAllActions('SceneEnded',false);
		if ( !thePlayer.IsInCombat() )
			thePlayer.SetPlayerCombatStance( PCS_Normal );
	}
	
	
	public function SetDeathType( type : EPlayerDeathType )
	{
		SetBehaviorVariable( 'DeathType', (float) (int) type );
	}
	
	public function ResetDeathType()
	{
		SetDeathType( PDT_Normal );
	}
	
	
	event OnDeath( damageAction : W3DamageAction  )
	{
		var attacker : CGameplayEntity;
		var hud : CR4ScriptedHud;
		var radialModule : CR4HudModuleRadialMenu;
		var depth : float;
		var guiManager : CR4GuiManager;
		
		var allowDeath : bool;
		
		super.OnDeath( damageAction );
		
		ClearAttitudes( true, false, false );
		
		attacker = damageAction.attacker;
		
		depth = ((CMovingPhysicalAgentComponent)this.GetMovingAgentComponent()).GetSubmergeDepth();
		
		if ( (W3ReplacerCiri)this )
		{	
			allowDeath = true;
		}
		else if ( !IsUsingVehicle() && depth > -0.5 && !IsSwimming() && attacker && ((CNewNPC)attacker).GetNPCType() == ENGT_Guard )
		{
			((CR4PlayerStateUnconscious)GetState('Unconscious')).OnKilledByGuard();
			PushState( 'Unconscious' );
		}
		else if ( !IsUsingVehicle() && depth > -0.5 && !IsSwimming() && (W3Elevator)attacker )
		{
			((CR4PlayerStateUnconscious)GetState('Unconscious')).OnKilledByElevator();
			PushState( 'Unconscious' );
		}
		else if ( !IsUsingVehicle() && depth > -0.5 && !IsSwimming() && WillBeUnconscious() )
		{
			PushState( 'Unconscious' );
		}
		else
		{
			allowDeath = true;
		}
		
		if ( allowDeath )
		{
			if(IsAlive())
			{
				SetAlive(false);
			}
			
			if ( IsUsingHorse( true ) || IsUsingBoat() )
			{
			}
			else
			{
				RaiseForceEvent( 'Death' );
				
				
				theSound.EnterGameState(ESGS_Death);
				theSound.SoundEvent( 'gui_global_player_death_thump' );
				
				theGame.SetTimeScale(0.6f, theGame.GetTimescaleSource(ETS_DebugInput), theGame.GetTimescalePriority(ETS_DebugInput), false, true );
				
				
				
				
				
				
				
			}
			
			theGame.FadeOutAsync(DEATH_SCREEN_OPEN_DELAY - 0.1 );
			
			hud = (CR4ScriptedHud)theGame.GetHud();
			
			guiManager = theGame.GetGuiManager();
			if (guiManager && guiManager.IsAnyMenu())
			{
				guiManager.GetRootMenu().CloseMenu();
			}
			
			
			
			if (hud)
			{
				hud.StartDeathTimer(DEATH_SCREEN_OPEN_DELAY);
			}
			else
			{
				AddTimer('OpenDeathScreen',DEATH_SCREEN_OPEN_DELAY,false); 
			}
			
			
			if( hud )
			{
				radialModule = (CR4HudModuleRadialMenu)hud.GetHudModule("RadialMenuModule");
				if (radialModule && radialModule.IsRadialMenuOpened())
				{
					radialModule.HideRadialMenu();
				}
			}
			theTelemetry.LogWithLabel(TE_FIGHT_PLAYER_DIES, damageAction.attacker.ToString());
		}
	}
	
	
	timer function JumpOnRagdollFix( deltaTime : float , id : int)
	{
		TurnOnRagdoll();
	}
	
	
	
	
	
	
	
	


	event OnUnconsciousEnd()
	{
		if( GetCurrentStateName() == 'Unconscious' )
		{
			GotoStateAuto();
		}
	}

	
	
	
	event OnDodgeBoost(){}
		
	
	function StopRun()
	{
		SetSprintActionPressed(false,true);
		SetIsSprinting( false );
	}
	
	function IsRunPressed() : bool
	{
		return true;
		
	}
	
	
	private var sprintButtonPressedTimestamp : float;
	
	function SetSprintActionPressed( enable : bool, optional dontClearTimeStamp : bool )
	{
		sprintActionPressed = enable;
		if ( !dontClearTimeStamp )
			sprintButtonPressedTimestamp = theGame.GetEngineTimeAsSeconds();
	}
	
	public function GetHowLongSprintButtonWasPressed() : float
	{
		var duration : float;
		
		if ( !sprintActionPressed || sprintButtonPressedTimestamp <= 0)
			return -1;
		
		duration = theGame.GetEngineTimeAsSeconds() - sprintButtonPressedTimestamp;
		
		return duration;
	}
	
	function SetIsSprinting( flag : bool )
	{
		
		if( flag == isSprinting )
		{
			if ( flag && disableSprintingTimerEnabled )
			{
				disableSprintingTimerEnabled = false;
				RemoveTimer( 'DisableSprintingTimer' );
			}
			return;
		}
		
		if ( flag )
		{
			theGame.GetBehTreeReactionManager().CreateReactionEventIfPossible( thePlayer, 'PlayerSprintAction', -1, 10.0f, 0.5f, -1, true); 
			BreakPheromoneEffect();
			RemoveTimer( 'DisableSprintingTimer' );
			AddTimer('SprintingTimer', 0.01, true);
			
		}
		else 
		{
			sprintingTime = 0.0f;
			theGame.GetBehTreeReactionManager().RemoveReactionEvent( thePlayer, 'PlayerSprintAction' ); 
			ResumeStaminaRegen( 'Sprint' );
			EnableSprintingCamera( false );
		}
		
		isSprinting = flag;	
		SetBehaviorVariable( 'isSprinting', (int)isSprinting );
	}
	
	var sprintingCamera : bool;
	function EnableSprintingCamera( flag : bool )
	{
		var camera 	: CCustomCamera;
		var animation : SCameraAnimationDefinition;
		var vel : float;

		if( !theGame.IsUberMovementEnabled() && !useSprintingCameraAnim )
		{
			return;
		}

		if ( IsSwimming() || OnCheckDiving() )
			flag = false;
		
		camera = theGame.GetGameCamera();
		if ( flag )
		{
			
			vel = VecLength( this.GetMovingAgentComponent().GetVelocity() );
			if ( !sprintingCamera && vel > 6.5 )
			{
				if( useSprintingCameraAnim )
				{
					animation.animation = 'camera_shake_loop_lvl1_1';
					animation.priority = CAP_High;
					animation.blendIn = 1.f;
					animation.blendOut = 1.f;
					animation.weight = 1.5f;
					animation.speed	= 1.0f;
					animation.loop = true;
					animation.additive = true;
					animation.reset = true;
					camera.PlayAnimation( animation );
				}
				
				sprintingCamera = true;
			}
		}
		else
		{	
			sprintingCamera = false;
			camera.StopAnimation('camera_shake_loop_lvl1_1');
		}
	}
	
	var runningCamera : bool;
	function EnableRunCamera( flag : bool )
	{
		var camera 	: CCustomCamera = theGame.GetGameCamera();
		var animation : SCameraAnimationDefinition;
		var vel : float;
		
		if ( IsSwimming() || OnCheckDiving() )
			flag = false;
		
		if ( flag )
		{
			animation.animation = 'camera_shake_loop_lvl1_5';
			animation.priority = CAP_High;
			animation.blendIn = 1.f;
			animation.blendOut = 1.f;
			animation.weight = 0.7f;
			animation.speed	= 0.8f;
			animation.loop = true;
			animation.additive = true;
			animation.reset = true;
			camera.PlayAnimation( animation );
		}
		else
		{
			camera.StopAnimation('camera_shake_loop_lvl1_5');
		}
		
		runningCamera = flag;
	}
	
	
	protected timer function SprintingTimer(dt : float, id : int)
	{
		if ( !thePlayer.modifyPlayerSpeed )
		{
			sprintingTime	+= dt;
			
			
			if ( ShouldDrainStaminaWhileSprinting() )
			{
				DrainStamina(ESAT_Sprint, 0, 0, '', dt);
			}
		}
	}
	
	protected function ShouldDrainStaminaWhileSprinting() : bool
	{
		var currentStateName : name;
		
		if ( sprintingTime >= 3.0 || GetStaminaPercents() < 1.0 )
		{
			currentStateName = GetCurrentStateName();
			
			
			if(  currentStateName == 'CombatSteel' || currentStateName == 'CombatSilver' || currentStateName == 'CombatFists' )
			{
				return true;
			}
		}
		return false;
	}
	
	protected function ShouldUseStaminaWhileSprinting() : bool
	{
		return true;
	}
	
	function GetIsSprinting() : bool
	{
		return isSprinting;
	}
	
	function GetSprintingTime() : float
	{
		if( !GetIsSprinting() )
		{
			return 0.0f;
		}
		
		return sprintingTime;
	}
	
	
	var disableSprintingTimerEnabled	: bool;
	timer function DisableSprintingTimer ( time : float , id : int)
	{
		disableSprintingTimerEnabled = false;
		if ( !thePlayer.CanSprint( VecLength( rawLeftJoyVec ) ) )
		{
			thePlayer.RemoveTimer('SprintingTimer');
			thePlayer.SetIsSprinting(false);
		}
	}
	
	public function IsSprintActionPressed() : bool
	{
		
		if(thePlayer.GetLeftStickSprint() && theInput.LastUsedGamepad())
		{
			return GetIsSprintToggled();
		}
		else
		{
			return theInput.IsActionPressed('Sprint') || sprintToggle;
		}
		
	}
	
	public function SetSprintToggle( flag : bool )
	{	
		sprintToggle = flag;
	}
	
	public function GetIsSprintToggled() : bool
	{	
		return sprintToggle;
	}

	public function SetWalkToggle( flag : bool )
	{	
		walkToggle = flag;
	}
	
	public function GetIsWalkToggled() : bool
	{	
		return walkToggle;
	}
	
	public function GetIsRunning() : bool
	{
		return isRunning;
	}
	
	public function SetIsRunning( flag : bool )
	{
		isRunning = flag;
	}
	
	function GetIsWalking() : bool
	{
		return isWalking;
	}
	
	function SetIsWalking( walking : bool )
	{
		isWalking	= walking;
	}
	
	final function SetIsMovable( flag : bool )
	{
		isMovable = flag;
	}
	
	public function SetManualControl( movement : bool, camera : bool ) 
	{ 
		if( movement == false )
		{
			RaiseForceEvent( 'Idle' );
		}
		SetIsMovable( movement ); 
		SetShowHud( movement );
	}
	
	final function GetIsMovable() : bool
	{
		return isMovable && inputHandler.IsActionAllowed(EIAB_Movement);
	}
	
	function SetBInteractionPressed( flag : bool )
	{
		bInteractionPressed = flag;
	}
	
	function GetBInteractionPressed() : bool 
	{
		return bInteractionPressed;
	}
	
	function IsInCombatAction()  : bool
	{
		return bIsInCombatAction;
	}
	
	function IsInCombatActionFriendly()  : bool
	{
		return bIsInCombatActionFriendly;
	}
	
	public function IsInCombatAction_SpecialAttack() : bool
	{
		return false;
	}
	
	public function SetBIsInCombatAction(flag : bool)
	{
		if( flag )
		{
			thePlayer.SetBehaviorVariable( 'inJumpState', 1.f );
			
		}
		else
		{
			thePlayer.SetBehaviorVariable( 'inJumpState', 0.f );
			
		}
		
		bIsInCombatAction = flag;
		SetBehaviorVariable( 'isInCombatActionForOverlay', (float)bIsInCombatAction );
		
	}
	
	public function SetBIsInCombatActionFriendly(flag : bool)
	{
		bIsInCombatActionFriendly = flag;
	}
	
	public function RaiseCombatActionFriendlyEvent() : bool
	{
		if ( CanRaiseCombatActionFriendlyEvent() )
		{
			if( RaiseEvent('CombatActionFriendly') )
			{
				SetBIsInCombatActionFriendly( true ); 
				return true;
			}
		}
		
		return false;
	}
	
	public function CanRaiseCombatActionFriendlyEvent( optional isShootingCrossbow : bool ) : bool
	{
		var raiseEvent 		: bool = false;
		var playerWitcher 	: W3PlayerWitcher;
		var itemId 			: SItemUniqueId;
		
		playerWitcher = (W3PlayerWitcher)this;
	
		if ( !playerWitcher )
			return true;
		else if ( isShootingCrossbow )
			return true;
		else if ( thePlayer.IsOnBoat() && !thePlayer.IsCombatMusicEnabled() )
			return true; 
		else
		{
			itemId = thePlayer.GetSelectedItemId();
			if ( !( playerWitcher.IsHoldingItemInLHand() && inv.IsIdValid(itemId) && !inv.IsItemCrossbow(itemId) && !inv.IsItemBomb(itemId) ) )
 				return true;
		}
			
		thePlayer.DisplayActionDisallowedHudMessage( EIAB_Undefined,,, true );	
		return false;		
	}
	
	
	final function CanParryAttack() : bool
	{		
		return inputHandler.IsActionAllowed(EIAB_Parry) && ParryCounterCheck() && !IsCurrentlyDodging() && super.CanParryAttack(); 
	}
	
	
	protected function ParryCounterCheck() : bool
	{
		var combatActionType  : int;
		combatActionType = (int)GetBehaviorVariable( 'combatActionType'); 
		
		if ( combatActionType == (int)CAT_Parry )
			return true;
			
		if ( GetBIsCombatActionAllowed() )
			return true;
			
		if ( thePlayer.IsInCombatAction() && combatActionType == (int)CAT_Dodge )
		{
			if ( thePlayer.CanPlayHitAnim() && thePlayer.IsThreatened() )
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	function SetIsHorseMounted( isOn : bool )
	{
		isHorseMounted = isOn;
	}
	
	function GetIsHorseMounted() : bool
	{
		return isHorseMounted;
	}
	
	
	function SetIsCompanionFollowing( isOn : bool )
	{
		isCompanionFollowing = isOn;
	}
	function GetIsCompanionFollowing() : bool
	{
		return isCompanionFollowing;
	}

	function SetStartScreenIsOpened( isOpened : bool) : void 
	{
		bStartScreenIsOpened = isOpened;
		
		
		if( isOpened )
			theSound.EnterGameState( ESGS_MusicOnly );
		else
			theSound.LeaveGameState( ESGS_MusicOnly );
	}
	
	function GetStartScreenIsOpened( ) : bool 
	{
		return bStartScreenIsOpened;
	}
	
	function SetEndScreenIsOpened( isOpened : bool) : void 
	{
		bEndScreenIsOpened = isOpened;
		
		
		if( isOpened )
			theSound.EnterGameState( ESGS_MusicOnly );
		else
			theSound.LeaveGameState( ESGS_MusicOnly );
	}
	
	function GetEndScreenIsOpened( ) : bool 
	{
		return bEndScreenIsOpened;
	}

	function SetStartScreenFadeDuration( fadeTime : float) : void 
	{
		fStartScreenFadeDuration = fadeTime;
	}	

	function GetStartScreenFadeDuration( ) : float 
	{
		return fStartScreenFadeDuration;
	}
	
	function SetStartScreenFadeInDuration( fadeTime : float) : void 
	{
		fStartScreenFadeInDuration = fadeTime;
	}	

	function GetStartScreenFadeInDuration( ) : float 
	{
		return fStartScreenFadeInDuration;
	}
	
	function SetStartScreenEndWithBlackScreen( value : bool ) : void 
	{
		bStartScreenEndWithBlackScreen = value;
	}	
	
	function GetStartScreenEndWithBlackScreen( ) : bool 
	{
		return bStartScreenEndWithBlackScreen;
	}
	
	
	
	public function CanStartTalk() : bool
	{
		var stateName : name;
		stateName = thePlayer.GetCurrentStateName();
		return ( stateName != 'CombatSteel' && stateName != 'CombatSilver' && stateName != 'CombatFists' );	
	}
		
	
	
	

	public function UpdateRequestedDirectionVariables_PlayerDefault()
	{
		UpdateRequestedDirectionVariables( rawPlayerHeading, GetHeading() );
	}
	
	
	
	
	
	function SetGuarded( flag : bool )
	{
		super.SetGuarded(flag);
		SetParryEnabled(IsGuarded());
		
		if ( !thePlayer.IsInCombat() )
		{
			if ( flag )
				OnDelayOrientationChange();
			else
				thePlayer.EnableManualCameraControl( true, 'Guard' );
		}
		
		
	}
	
	
	
	
	
	
	event OnDelayOrientationChange();
	
	function SetBIsInputAllowed( flag : bool, sourceName : name )
	{
		bIsInputAllowed = flag;
		
		if(flag)
		{
			debug_BIsInputAllowedLocks.Clear();
		}
		else
		{
			debug_BIsInputAllowedLocks.PushBack(sourceName);
		}
	}
	
	function GetBIsInputAllowed() : bool
	{
		return bIsInputAllowed;
	}
	
	function SetBIsFirstAttackInCombo( flag : bool )
	{
		bIsFirstAttackInCombo = flag;
	}
	
	function IsInHitAnim() : bool
	{
		return bIsInHitAnim;
	}
	
	function SetIsInHitAnim( flag : bool )
	{
		bIsInHitAnim = flag;
	}	
	
	function SetInputModuleNeededToRun( _inputModuleNeededToRun : float )
	{
		inputModuleNeededToRun = ClampF(_inputModuleNeededToRun, 0.5f, 1.f);
	}
	
	function GetInputModuleNeededToRun() : float
	{
		var configValue:string;
		
		if (inputModuleNeededToRun == -1.0)
		{
			configValue = ((CInGameConfigWrapper)theGame.GetInGameConfigWrapper()).GetVarValue('Controls', 'LeftStickSensitivity');
			inputModuleNeededToRun = StringToFloat(configValue, 0.7);
		}
		
		return inputModuleNeededToRun;
	}
	

	
	event OnAnimEvent_AllowInput( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		if ( animEventType == AET_DurationStart )
		{
			
			SetBIsInputAllowed( true, 'AnimEventAllowInputStart' );
		}
		
	}
	
	event OnAnimEvent_DisallowInput( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		if ( animEventType == AET_DurationStart )
		{
			SetBIsInputAllowed( false, 'AnimEventDisallowInputStart' );
		}
		else if ( animEventType == AET_DurationEnd )
		{
			SetBIsInputAllowed( true, 'AnimEventDisallowInputEnd' );
		}
	}
	
	event OnAnimEvent_DisallowHitAnim( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		if ( animEventType == AET_DurationEnd )
		{
			SetCanPlayHitAnim( true );	
		}
		else if ( ( GetBehaviorVariable( 'combatActionType' ) == (int)CAT_Attack && !this.bIsFirstAttackInCombo )
				|| ( GetBehaviorVariable( 'combatActionType' ) == (int)CAT_Dodge && GetBehaviorVariable( 'isRolling' ) == 0.f ) )
		{
		}
		else
		{
			SetCanPlayHitAnim( false );
		}
	}
	
	event OnAnimEvent_AllowHitAnim( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		SetCanPlayHitAnim( true );
	}
	
	event OnAnimEvent_AllowBlend( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		SetCanPlayHitAnim( true );	
	}
	
	event OnAnimEvent_QuickSlotItems( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		var itemEntity : W3UsableItem;
		
		if( animEventName == 'EquipItem' && currentlyUsedItem )
		{
			inv.MountItem( currentlyEquipedItem, true );
		}
		else if( animEventName == 'UseItem' && currentlyUsedItem )
		{
			currentlyUsedItem.OnUsed( this );
		}
		else if( animEventName == 'HideItem' )
		{
			inv.UnmountItem( currentlyEquipedItem, true );
			currentlyEquipedItem = GetInvalidUniqueId();
		}
		else if( animEventName == 'EquipItemL'  )
		{
			if ( thePlayer.IsHoldingItemInLHand() ) 
			{
				inv.MountItem( currentlyEquipedItemL, true );
				
				
				
				
				thePlayer.StartWaitForItemSpawnAndProccesTask();
			}
		}
		else if( ( animEventName == 'UseItemL' || animEventName == 'ItemUseL') )
		{
			
			
			thePlayer.AllowUseSelectedItem();
		}
		else if( animEventName == 'HideItemL' )
		{
			
			thePlayer.KillWaitForItemSpawnAndProccesTask();
		
			if ( currentlyUsedItemL )
			{
				currentlyUsedItemL.OnHidden( this );
				currentlyUsedItemL.SetVisibility( false );
			}
			inv.UnmountItem( currentlyEquipedItemL, true );
			
		}
	}
	
	event OnAnimEvent_SetRagdoll( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		if ( ( ( CMovingPhysicalAgentComponent ) this.GetMovingAgentComponent() ).HasRagdoll() )
		{
			if ( this == thePlayer && !thePlayer.IsOnBoat() )
			{
				TurnOnRagdoll();
				
			}
		}
	}
	
	
	event OnAnimEvent_InAirKDCheck( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		if(IsInAir())
		{
			TurnOnRagdoll();
		}
	}
	
	private var illusionMedallion : array<SItemUniqueId>;
	
	event OnAnimEvent_EquipMedallion( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		illusionMedallion.Clear();
		illusionMedallion = inv.GetItemsByName( 'Illusion Medallion' );
		inv.MountItem( illusionMedallion[0], true );
	}
	
	event OnAnimEvent_HideMedallion( animEventName : name, animEventType : EAnimationEventType, animInfo : SAnimationEventAnimInfo )
	{
		inv.UnmountItem( illusionMedallion[0], true );
		illusionMedallion.Clear();
	}
	


	
	event OnDiving(dir : int){}
	event OnDive(){}
	
	
	event OnCheckDiving()				{ return false; }
	event OnAllowShallowWaterCheck()	{ return true; }
	event OnCheckUnconscious()			{ return false; }
	event OnAllowSwimmingSprint()		{ return false; }
	event OnAllowedDiveDown()			{ return true; }
	event OnDiveInput( divePitch : float ){}
	
	event OnIsCameraUnderwater()
	{
		
		
		return false;
	}
	
			
	
	
	
	
	
	
	event OnHitGround()
	{
	}
	
	event OnHitCeiling()
	{		
	}
	
	
	
	
	
	
	
	private function SetTerrModifier( val : float )
	{
		SetBehaviorVariable( 'TerrainModifier', val );
		terrModifier = val;
	}
	
	private function SetTerrTypeOne( type : ETerrainType )
	{
		SetBehaviorVariable( 'TerrainType', (int)type );
		terrTypeOne = type;
	}
	
	private function SetTerrTypeTwo( type : ETerrainType )
	{
		SetBehaviorVariable( 'TerrainTypeBlended', (int)type );
		terrTypeTwo = type;
	}
	
	public function SteppedOnTerrain( type : ETerrainType )
	{	
		
		if( type != terrTypeOne && type != terrTypeTwo )
		{
			if( terrTypeOne == prevTerrType )
			{
				
				SetTerrTypeTwo( type );
				SetTerrModifier( 0.01f );
			}
			else if( terrTypeTwo == prevTerrType )
			{
				SetTerrTypeOne( type );
				SetTerrModifier( 0.99f );
			}
		}
		
		if( type == terrTypeOne )
		{
			terrModifier -= 0.1f;
		}
		else if( type == terrTypeTwo )
		{
			terrModifier += 0.1f;
		}
		
		terrModifier = ClampF( terrModifier, 0.01f, 0.99f );
		
		SetBehaviorVariable( 'TerrainModifier', terrModifier );
		
		prevTerrType = type;
	}
	
	
	
	
	
	
	
	function PlayerCanComment() : bool
	{
		var time : EngineTime;
		time = commentaryLastTime + commentaryCooldown;
		
		return theGame.GetEngineTime() > time;
	}
	
	function PlayerCanPlayMonsterCommentary() : bool
	{
		var time : EngineTime;
		var commentaryMonsterCooldown : float;
		
		commentaryMonsterCooldown = 120.0f;
		time = commentaryLastTime + commentaryMonsterCooldown;
		
		return theGame.GetEngineTime() > time;
	}
	
	function PlayerCommentary( commentaryType : EPlayerCommentary, optional newCommentaryCooldown : float ) 
	{
		var actor		: CPlayer = thePlayer;
		var activeActor : CEntity;
		var hud : CR4ScriptedHud;

		hud = (CR4ScriptedHud)theGame.GetHud();
		activeActor = (CEntity) actor;
		
		commentaryLastTime = theGame.GetEngineTime();
		
		if( newCommentaryCooldown > 0.0f )
		{
			commentaryCooldown = newCommentaryCooldown;
		}
		else
		{
			commentaryCooldown = 20.0f;
		}
		if( commentaryType == PC_MedalionWarning  )
		{
			PlayVoiceset( 1, "warning"  );
			hud.ShowOneliner( "My medallion", activeActor );
			AddTimer( 'TurnOffOneliner', 3.5f );
			
		}
		else if( commentaryType == PC_MonsterReaction  )
		{
			PlayVoiceset( 1, "monster" );
		}
		
		else if( commentaryType == PC_ColdWaterComment )
		{
			
			hud.ShowOneliner( "Damn, it's cold!", activeActor );
			AddTimer( 'TurnOffOneliner', 3.5f );
		}
	}
	
	timer function TurnOffOneliner( deltaTime : float , id : int)
	{
		var hud : CR4ScriptedHud;
		hud = (CR4ScriptedHud)theGame.GetHud();
		hud.HideOneliner( this );
	}
	
	
	
	
	
	
	
	public function CanPlaySpecificVoiceset() : bool 					{ return canPlaySpecificVoiceset; }
	public function SetCanPlaySpecificVoiceset( val : bool ) 			{ canPlaySpecificVoiceset = val; }
	timer function ResetSpecificVoicesetFlag( dt : float, id : int )	{ SetCanPlaySpecificVoiceset( true ); }
	
	
	private var numberOfEnemiesAttacking : int;
	final function SetPlayerUnderAttack( toggle : bool )
	{
		if( toggle )
		{
			numberOfEnemiesAttacking += 1;
		}
		else
		{
			numberOfEnemiesAttacking -= 1;
		}
	}
	
	final function IsPlayerUnderAttack() : bool
	{
		return numberOfEnemiesAttacking;
	}
	
	
	
	function GetThreatLevel() : int
	{
		return 5;
	}
	
	function GetBIsCombatActionAllowed() : bool
	{
		return true;
	}
	
	import function SetEnemyUpscaling( b : bool );
	import public function GetEnemyUpscaling() : bool;
	
	public function SetAutoCameraCenter( on : bool ) { autoCameraCenterToggle = on; }
	public function GetAutoCameraCenter() : bool
	{
		return autoCameraCenterToggle || IsCameraLockedToTarget();
	}

	public function SetVehicleCachedSign( sign : ESignType ) { vehicleCachedSign = sign; }
	public function GetVehicleCachedSign() : ESignType { return vehicleCachedSign; }
		
	public function GetMoney() : int
	{
		return inv.GetMoney();
	}
	
	public function AddMoney(amount : int) 
	{
		inv.AddMoney(amount);
	}
	
	public function RemoveMoney(amount : int) 
	{
		inv.RemoveMoney(amount);
	}

	function GetThrowItemMode() : bool 
	{
		return false;
	}
	
	function GetEquippedSign() : ESignType
	{
		return ST_None;
	}
	
	function GetCurrentlyCastSign() : ESignType
	{
		return ST_None;
	}
	
	function IsCastingSign() : bool
	{
		return false;
	}
	
	function IsCurrentSignChanneled() : bool
	{
		return false;
	}
	
	
	function OnRadialMenuItemChoose( selectedItem : string )
	{
		
	}
	
	
	public function UpdateQuickSlotItems() : bool 
	{
		return false;
	}
	
	
	public function SetUpdateQuickSlotItems(bUpdate : bool )  
	{
		
	}
	
	public function RemoveAllPotionEffects(optional skip : array<CBaseGameplayEffect>)
	{
		effectManager.RemoveAllPotionEffects(skip);
	}
	
	public function BreakPheromoneEffect() : bool
	{
		if( thePlayer.HasBuff( EET_PheromoneNekker ) || thePlayer.HasBuff( EET_PheromoneDrowner ) || thePlayer.HasBuff( EET_PheromoneBear ) )
		{
			thePlayer.RemoveBuff( EET_PheromoneNekker );
			thePlayer.RemoveBuff( EET_PheromoneDrowner );
			thePlayer.RemoveBuff( EET_PheromoneBear );
		}
		
		return true;
	}
	
	
		
	public function GetCurrentTrackedQuestSystemObjectives() : array<SJournalQuestObjectiveData>
	{
		return currentTrackedQuestSystemObjectives;
	}

	public function SetCurrentTrackedQuestSystemObjectives(cTQO : array<SJournalQuestObjectiveData>) : void
	{
		var i : int;
		
		currentTrackedQuestSystemObjectives = cTQO;
		
		for(i = 0; i < cTQO.Size(); i+=1)
		{
			currentTrackedQuestSystemObjectives[i] = cTQO[i];
		}
	}
	
	public function GetCurrentTrackedQuestObjectives() : array<SJournalQuestObjectiveData>
	{
		return currentTrackedQuestObjectives;
	}

	public function SetCurrentTrackedQuestObjectives(cTQO : array<SJournalQuestObjectiveData>) : void
	{
		var i : int;
	
		currentTrackedQuestObjectives = cTQO;
		
		for(i = 0; i < cTQO.Size(); i+=1)
		{
			currentTrackedQuestObjectives[i] = cTQO[i];
		}
	}

	public function GetCurrentTrackedQuestGUID() : CGUID
	{
		return currentTrackedQuestGUID;
	}
	
	public function SetCurrentTrackedQuestGUID(cTQG : CGUID) : void
	{
		currentTrackedQuestGUID = cTQG;
	}
	
	
	public function HAXCheckIfNew(checkGUID : CGUID ):bool
	{
		var i : int;
		for( i = 0; i < HAXNewObjTable.Size(); i += 1)
		{
			if( HAXNewObjTable[i] == checkGUID)
			{
				return false;
			}
		}
		
		HAXNewObjTable.PushBack(checkGUID);
		return true;
	}
			
	public function GetShowHud() : bool 
	{
		return true;
	}

	public function SetShowHud( value : bool ) : void
	{
		
	}
	
	
	
	
	
	function DebugKillAll()
	{
		var i, enemiesSize : int;
		var actors : array<CActor>;
		
		actors = GetNPCsAndPlayersInRange(20, 20, '', FLAG_Attitude_Hostile);
		enemiesSize = actors.Size();
		
		for( i = 0; i < enemiesSize; i += 1 )
			actors[i].Kill( 'Debug', false, this);					
	}
	
	public function DebugTeleportToPin( optional posX : float , optional posY : float )
	{
		var mapManager 		: CCommonMapManager = theGame.GetCommonMapManager();
		var rootMenu		: CR4Menu;
		var mapMenu			: CR4MapMenu;
		var currWorld		: CWorld = theGame.GetWorld();
		var destWorldPath	: string;
		var id				: int;
		var area			: int;
		var type			: int;
		var position		: Vector;
		var rotation 		: EulerAngles;
		var goToCurrent		: Bool = false;
		
		rootMenu = (CR4Menu)theGame.GetGuiManager().GetRootMenu();
		
		if ( rootMenu )
		{
			mapMenu = (CR4MapMenu)rootMenu.GetSubMenu();
			
			if ( mapMenu )
			{
				position.X = posX;
				position.Y = posY;
				destWorldPath = mapManager.GetWorldPathFromAreaType( mapMenu.GetShownMapType() );
				
				if ( mapMenu.IsCurrentAreaShown() )
				{
					goToCurrent = true;
				}
				
				rootMenu.CloseMenu();
			}
		}
		else
		{	
			mapManager.GetUserMapPinByIndex( 0, id, area, position.X, position.Y, type );		
			destWorldPath = mapManager.GetWorldPathFromAreaType( area );
			
			if (destWorldPath == "" || destWorldPath == currWorld.GetPath() )
			{
				goToCurrent = true;
			}
		}
		
		if ( goToCurrent )
		{
			currWorld.NavigationComputeZ(position, -500.f, 500.f, position.Z);
			currWorld.NavigationFindSafeSpot(position, 0.5f, 20.f, position);
				
			Teleport( position );
		
			if ( !currWorld.NavigationComputeZ(position, -500.f, 500.f, position.Z) )		
			{
				AddTimer( 'DebugWaitForNavigableTerrain', 1.f, true );
			}
		}
		else
		{
			theGame.ScheduleWorldChangeToPosition( destWorldPath, position, rotation );
			AddTimer( 'DebugWaitForNavigableTerrain', 1.f, true, , , true );
		}
	}
	
	timer function DebugWaitForNavigableTerrain( delta : float, id : int )
	{
		var position 	: Vector = GetWorldPosition();
		
		if ( theGame.GetWorld().NavigationComputeZ(position, -1000.f, 1000.f, position.Z) )
		{
			RemoveTimer( 'DebugWaitForNavigableTerrain' );
			theGame.GetWorld().NavigationFindSafeSpot(position, 0.5f, 20.f, position);
			Teleport( position );
		}
	}
	
	event OnHitByObstacle( obstacleComponent : CComponent )
	{
		obstacleComponent.SetEnabled( false );
	}
	
	public function DEBUGGetDisplayRadiusMinimapIcons():bool 
	{
		return _DEBUGDisplayRadiusMinimapIcons;
	}

	public function DEBUGSetDisplayRadiusMinimapIcons(inValue : bool):void 
	{
		_DEBUGDisplayRadiusMinimapIcons = inValue;
	}
	
	public function Dbg_UnlockAllActions()
	{
		inputHandler.Dbg_UnlockAllActions();
	}
		
	
	
	
	
	event OnCriticalStateAnimStopGlobalHack()
	{
		var buff : CBaseGameplayEffect;
		
		if(!csNormallyStoppedBuff)
		{
			if(effectManager)
			{
				buff = effectManager.GetCurrentlyAnimatedCS();
				if(buff)
					OnCriticalStateAnimStop();
			}						
		}
		else
		{
			csNormallyStoppedBuff = false;
		}
	}
	
	private var csNormallyStoppedBuff : bool;
	
	
	event OnCriticalStateAnimStop()
	{
		csNormallyStoppedBuff = true;
			
		SetBehaviorVariable( 'bCriticalState', 0);
		CriticalStateAnimStopped(false);
		if ( this.IsRagdolled() ) 
			this.RaiseForceEvent('RecoverFromRagdoll');
		return true;
	}
	
	event OnRecoverFromRagdollEnd()
	{
		if ( this.IsRagdolled() ) 
			this.SetKinematic(true);
	}
	
	public function ReapplyCriticalBuff() 
	{
		var buff : CBaseGameplayEffect;
		
		
		buff = ChooseCurrentCriticalBuffForAnim();
		if(buff)
		{
			LogCritical("Reapplying critical <<" + buff.GetEffectType() + ">> after finished CombatAction (End)");
			StartCSAnim(buff);
		}
	}
	
	timer function ReapplyCSTimer(dt : float, id : int)
	{
		ReapplyCriticalBuff();
	}
	
	public function IsInAgony() : bool					{return false;}
	
	public function GetOTCameraOffset() : float
	{
		return oTCameraOffset;
	}
	
	public function IsKnockedUnconscious() : bool	
	{
		return OnCheckUnconscious();
	}
	
	
	
	
	function IsSailing() : bool
	{
		return IsUsingVehicle() && GetCurrentStateName() == 'Sailing';
	}
	
	final function spawnBoatAndMount()
	{
		var entities : array<CGameplayEntity>;
		var vehicle : CVehicleComponent;
		var i : int;
		var boat : W3Boat;
		var ent : CEntity;
		var player : Vector;
		var rot : EulerAngles;
		var template : CEntityTemplate;
		
		FindGameplayEntitiesInRange( entities, thePlayer, 10, 10, 'vehicle' );
		
		for( i = 0; i < entities.Size(); i = i + 1 )
		{
			boat = ( W3Boat )entities[ i ];
			if( boat )
			{
				vehicle = ( CVehicleComponent )( boat.GetComponentByClassName( 'CVehicleComponent' ) );
				if ( vehicle )
				{
					vehicle.Mount( thePlayer, VMT_ImmediateUse, EVS_driver_slot );
				}
				
				return;
			}
		}

		rot = thePlayer.GetWorldRotation();	
		player = thePlayer.GetWorldPosition();
		template = (CEntityTemplate)LoadResource( 'boat' );
		player.Z = 0.0f;

		ent = theGame.CreateEntity(template, player, rot, true, false, false, PM_Persist );
		
		if( ent )
		{
			vehicle = ( CVehicleComponent )( ent.GetComponentByClassName( 'CVehicleComponent' ) );
			if ( vehicle )
			{
				vehicle.Mount( thePlayer, VMT_ImmediateUse, EVS_driver_slot );
				boat = ( W3Boat )ent;
				if( boat )
				{
					boat.SetTeleportedFromOtherHUB( true );
				}
			}
		}
	}
	
	timer function DelayedSpawnAndMountBoat( delta : float, id : int )
	{
		spawnBoatAndMount();
		RemoveTimer( 'DelayedSpawnAndMountBoat' );
	}
}
