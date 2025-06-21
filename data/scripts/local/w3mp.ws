struct W3mPlayerState
{
    var angles : EulerAngles;
    var position : Vector;
    var velocity : Vector;
    var speed : float;
    var moveType : int;
}

struct W3mPlayer
{
    var guid : Uint64;
    var playerName : string;
    var playerState : W3mPlayerState;
}

import function W3mSetNpcDisplayName(npc : CNewNPC, npcName : string);
import function W3mStorePlayerState(playerState : W3mPlayerState);
import function W3mGetPlayerStates() : array<W3mPlayer>;
import function W3mUpdatePlayerName(playerName : string);

function ConvertPlayerMoveType(playerMoveType : EPlayerMoveType) : int
{
    switch( playerMoveType)
    {
        case PMT_Idle:
            return 0;
            break;
            
        case PMT_Walk:
            return 1;
            break;		

        case PMT_Run:
            return 2;
            break;	
            
        case PMT_Sprint:
            return 3;
            break;

        default:
            return 0;
            break;
    }
}

function ConvertToMoveType(moveType : int) : EMoveType
{
    switch(moveType)
    {
        case 0:
            return MT_Walk;
            break;
            
        case 1:
            return MT_Walk;
            break;		

        case 2:
            return MT_Run;
            break;	
            
        case 3:
            return MT_Sprint;
            break;

        default:
            return MT_Walk;
            break;
    }
}

function TransmitPlayerState(actor : CActor)
{
    var playerState : W3mPlayerState;
    var movingAgent : CMovingAgentComponent;

    movingAgent = actor.GetMovingAgentComponent();

    playerState.position = actor.GetWorldPosition();
    playerState.angles = actor.GetWorldRotation();
    playerState.velocity = movingAgent.GetVelocity();
    playerState.speed = movingAgent.GetSpeed();
    playerState.moveType = ConvertPlayerMoveType(thePlayer.playerMoveType);

    W3mStorePlayerState(playerState);
}

function TransmitCurrentPlayerState()
{
    TransmitPlayerState((CActor)thePlayer);
}

function ApplyPlayerState(actor : CActor, player : W3mPlayer)
{
    var inter : int;
    var movingAgent : CMovingPhysicalAgentComponent;
    var actorPos : Vector;
    var targetPos : Vector;
    var playerState : W3mPlayerState;

    playerState = player.playerState;

    inter = 5;

    W3mSetNpcDisplayName((CNewNPC)actor, player.playerName);

    actorPos = actor.GetWorldPosition();
    targetPos = playerState.position;

    if(AbsF(actorPos.Z -  targetPos.Z) < 0.25)
    {
        targetPos.Z = actorPos.Z;
    }

    movingAgent = (CMovingPhysicalAgentComponent)actor.GetMovingAgentComponent();

    if(VecDistance(actor.GetWorldPosition(), playerState.position) > 0.5)
    {
        actor.TeleportWithRotation(targetPos, playerState.angles);
    }

    movingAgent.ApplyVelocity(playerState.velocity);

    if(playerState.speed > 1)
    {
        movingAgent.SetMoveType(MT_Run);
    }
    else
    {
        movingAgent.SetMoveType(MT_Walk);
    }

    movingAgent.SetGameplayMoveDirection(VecHeading(RotForward(playerState.angles)));
    movingAgent.SetGameplayRelativeMoveSpeed(playerState.speed * 0.6);
}

function AddAndEquip(npc: CNewNPC, item: name) {
    var ids : array<SItemUniqueId>;

    ids = npc.GetInventory().AddAnItem(item, 1);
    npc.EquipItem(ids[0]);
}

function CreateNewPlayerEntity() : CEntity
{
    var pos : Vector;
    var rot : EulerAngles;
    var ent : CEntity;
    var npc : CNewNPC;
    var template : CEntityTemplate;
    var followerMovingagent : CMovingAgentComponent;
    var followOnFootAI : CAIFollowSideBySideAction;
    var tags : array<name>;


    tags.PushBack('w3m_Player');

    rot = thePlayer.GetWorldRotation();	
    pos = thePlayer.GetWorldPosition();

    template = (CEntityTemplate)LoadResource("characters/npc_entities/main_npc/geralt_npc.w2ent", true);

    ent = theGame.CreateEntity(template, pos, rot,,,,,tags);
    npc = (CNewNPC)ent;

    AddAndEquip(npc, 'Autogen steel sword');
    AddAndEquip(npc, 'Autogen silver sword');
    AddAndEquip(npc, 'Autogen Pants');
    AddAndEquip(npc, 'Autogen Gloves');
    AddAndEquip(npc, 'Autogen Boots');
    AddAndEquip(npc, 'Autogen Armor');
    AddAndEquip(npc, 'Shaved With Tail Hairstyle');
    AddAndEquip(npc, 'head_3');

    npc.AddAbility('_canBeFollower', true); 

    followerMovingagent = npc.GetMovingAgentComponent();
    followerMovingagent.SetGameplayRelativeMoveSpeed(0.0f);
    followerMovingagent.SetDirectionChangeRate(0.16);
    followerMovingagent.SetMaxMoveRotationPerSec(60);
    
    npc.GotoState('NewIdle', false);

    W3mSetNpcDisplayName(npc, "W3M Player");

    npc.SetAttitude(thePlayer, AIA_Neutral);

    return ent;
}

statemachine class W3mStateMachine extends CEntity
{
    editable var players : array<CEntity>;

    public function start()
    {
        this.GotoState('MultiplayerState');
    }
}

state MultiplayerState in W3mStateMachine
{
    event OnEnterState(previous_state_name: name)
    {
        this.EntryFunction();
    }

    entry function EntryFunction()
    {
        this.RunMultiplayer();
    }

    latent function RunMultiplayer()
    {
        while(true)
        {
            RunMultiplayerFrame();
            Sleep(0.03);
        }
    }

    latent function RunMultiplayerFrame()
    {
        TransmitCurrentPlayerState();
        UpdateOtherPlayers();
    }

    latent function UpdateOtherPlayers()
    {
        var index : int;
        var player_states : array<W3mPlayer>;

        player_states = W3mGetPlayerStates();
        AdjustPlayers(player_states);

        for (index = 0; index < player_states.Size(); index += 1)
        {
            ApplyPlayerState((CActor)parent.players[index], player_states[index]);
        }
    }

    latent function AdjustPlayers(player_states : array<W3mPlayer>)
    {
        var current_player : CEntity;

        while (parent.players.Size() > player_states.Size())
        {
            current_player = parent.players[parent.players.Size() - 1];
            parent.players.Remove(current_player);

            current_player.Destroy();
        }

        while (parent.players.Size() < player_states.Size())
        {
            current_player = CreateNewPlayerEntity();
            parent.players.PushBack(current_player);
        }
    }
}

function DisplayFeedMessage(msg: string)
{
    var hud : CR4ScriptedHud;
    
    hud = (CR4ScriptedHud)theGame.GetHud();
    if (hud)
    {
        hud.HudConsoleMsg(msg);
    }
}

function DisplayCenterMessage(msg: string)
{
    GetWitcherPlayer().DisplayHudMessage(msg);
}

exec function SetName(playerName: string)
{
    W3mUpdatePlayerName(playerName);
    DisplayFeedMessage("Name changed: " + playerName);
}
