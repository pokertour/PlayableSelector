//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/Character", description: "Set character playable", color: "0 0 255 255", icon: HYBRID_COMPONENT_ICON)]
class PS_PlayableComponentClass : ScriptComponentClass
{
}

[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class PS_PlayableComponent : ScriptComponent
{
	[Attribute()]
	protected string m_name;
	[Attribute()]
	protected bool m_bIsPlayable;
	[Attribute()]
	protected ref array<ResourceName> m_aRespawnPrefabs;
	
	// Actually just RplId from RplComponent
	protected RplId m_id;
	protected vector spawnTransform[4];
	[RplProp()]
	protected int m_iRespawnCounter = 0;
	
	// Cache global
	protected PS_GameModeCoop m_GameModeCoop;
	protected PS_PlayableManager m_PlayableManager;
	
	// Cache components
	protected SCR_ChimeraCharacter m_Owner;
	SCR_ChimeraCharacter GetOwnerCharacter()
		return m_Owner;
	protected FactionAffiliationComponent m_FactionAffiliationComponent;
	FactionAffiliationComponent GetFactionAffiliationComponent()
		return m_FactionAffiliationComponent;
	protected SCR_EditableCharacterComponent m_EditableCharacterComponent;
	SCR_EditableCharacterComponent GetEditableCharacterComponent()
		return m_EditableCharacterComponent;
	protected SCR_UIInfo m_EditableUIInfo;
	SCR_UIInfo GetEditableUIInfo()
		return m_EditableUIInfo;
	protected SCR_CharacterDamageManagerComponent m_CharacterDamageManagerComponent;
	SCR_CharacterDamageManagerComponent GetCharacterDamageManagerComponent()
		return m_CharacterDamageManagerComponent;
	protected AIControlComponent m_AIControlComponent;
	AIControlComponent GetAIControlComponent()
		return m_AIControlComponent;
	protected AIAgent m_AIAgent;
	AIAgent GetAIAgent()
		return m_AIAgent;
	
	// Events
	protected ref ScriptInvokerInt m_eOnPlayerChange = new ScriptInvokerInt(); // int playerId
	ScriptInvokerInt GetOnPlayerChange()
		return m_eOnPlayerChange;
	void InvokeOnPlayerChanged(int playerId)
		m_eOnPlayerChange.Invoke(playerId);
	ScriptInvoker GetOnDamageStateChanged()
		return GetCharacterDamageManagerComponent().GetOnDamageStateChanged();
	protected ref ScriptInvokerVoid m_eOnUnregister = new ScriptInvokerVoid();
	ScriptInvokerVoid GetOnUnregister()
		return m_eOnUnregister;
	protected ref ScriptInvokerBase<SCR_BaseGameMode_OnPlayerDisconnected> m_eOnPlayerDisconnected = new ScriptInvokerBase<SCR_BaseGameMode_OnPlayerDisconnected>();
	ScriptInvokerBase<SCR_BaseGameMode_OnPlayerDisconnected> GetOnPlayerDisconnected()
		return m_eOnPlayerDisconnected;
	protected ref ScriptInvokerBase<SCR_BaseGameMode_PlayerId> m_eOnPlayerConnected = new ScriptInvokerBase<SCR_BaseGameMode_PlayerId>();
	ScriptInvokerBase<SCR_BaseGameMode_PlayerId> GetOnPlayerConnected()
		return m_eOnPlayerConnected;
	protected ref ScriptInvokerBase<SCR_BaseGameMode_OnPlayerRoleChanged> m_eOnPlayerRoleChange = new ScriptInvokerBase<SCR_BaseGameMode_OnPlayerRoleChanged>();
	ScriptInvokerBase<SCR_BaseGameMode_OnPlayerRoleChanged> GetOnPlayerRoleChange()
		return m_eOnPlayerRoleChange;
	protected ref ScriptInvokerInt m_eOnPlayerStateChange = new ScriptInvokerInt();
	ScriptInvokerInt GetOnPlayerStateChange()
		return m_eOnPlayerStateChange;
	protected ref ScriptInvokerBool m_eOnPlayerPinChange = new ScriptInvokerBool();
	ScriptInvokerBool GetOnPlayerPinChange()
		return m_eOnPlayerPinChange;
	
	// Temporally
	static protected int m_iRespawnTime;
	
	void CopyState(PS_PlayableComponent playable)
	{
		playable.SetPlayable(false);
		m_iRespawnCounter = playable.m_iRespawnCounter;
		m_aRespawnPrefabs = playable.m_aRespawnPrefabs;
		Replication.BumpMe();
	}
	
	override void OnPostInit(IEntity owner)
	{
		m_Owner = SCR_ChimeraCharacter.Cast(owner);
		m_Owner.PS_SetPlayable(this);
		GetGame().GetCallqueue().CallLater(AddToList, 0, false, owner); // init delay
		
		if (Replication.IsServer())
			owner.GetTransform(spawnTransform);
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	void GetSpawnTransform(vector outMat[4])
	{
		Math3D.MatrixCopy(spawnTransform, outMat);
	}
	
	ResourceName GetNextRespawn(bool nextPrefab)
	{
		if (!nextPrefab)
		{
			return m_Owner.GetPrefabData().GetPrefabName();
		}
		ResourceName prefab = "";
		if (!m_aRespawnPrefabs)
			return "";
		if (m_aRespawnPrefabs.Count() > m_iRespawnCounter)
			prefab = m_aRespawnPrefabs[m_iRespawnCounter];
		m_iRespawnCounter++;
		Replication.BumpMe();
		return prefab;
	}

	override void EOnInit(IEntity owner)
	{
		GetGame().GetCallqueue().Call(LateInit);
		
		m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
		m_EditableCharacterComponent = SCR_EditableCharacterComponent.Cast(owner.FindComponent(SCR_EditableCharacterComponent));
		m_EditableUIInfo = m_EditableCharacterComponent.GetInfo();
		m_CharacterDamageManagerComponent = SCR_CharacterDamageManagerComponent.Cast(owner.FindComponent(SCR_CharacterDamageManagerComponent));
		m_AIControlComponent = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		m_AIAgent = m_AIControlComponent.GetAIAgent();
	}
	
	void LateInit()
	{
		m_GameModeCoop = PS_GameModeCoop.Cast(GetGame().GetGameMode());
		m_PlayableManager = PS_PlayableManager.GetInstance();
	}

	// Get/Set Broadcast
	bool GetPlayable()
	{
		return m_bIsPlayable;
	}
	void SetPlayable(bool isPlayable)
	{
		RPC_SetPlayable(isPlayable);
		Rpc(RPC_SetPlayable, isPlayable);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetPlayable(bool isPlayable)
	{
		m_bIsPlayable = isPlayable;
		if (m_bIsPlayable) GetGame().GetCallqueue().CallLater(AddToList, 0, false, m_Owner);
		else RemoveFromList();
	}
	
	void OpenRespawnMenu(int time)
	{
		Rpc(RPC_OpenRespawnMenu, time);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_OpenRespawnMenu(int time)
	{
		m_iRespawnTime = time;
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PlayableRespawnMenu);
	}
	int GetRespawnTime()
		return m_iRespawnTime;

	void ResetRplStream()
	{
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		//rpl.EnableStreaming(true);
	}

	private void RemoveFromList()
	{
		GetGame().GetCallqueue().Remove(AddToList);
		GetGame().GetCallqueue().Remove(AddToListWrap);

		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return;

		if (m_Owner)
		{
			AIControlComponent aiComponent = AIControlComponent.Cast(m_Owner.FindComponent(AIControlComponent));
			if (aiComponent)
			{
				AIAgent agent = aiComponent.GetAIAgent();
				if (agent && m_bIsPlayable) agent.ActivateAI();
			}

			RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
			rpl.EnableStreaming(true);
		}

		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		playableManager.RemovePlayable(this);
		
		m_eOnUnregister.Invoke();
	}

	private void AddToList(IEntity owner)
	{
		if (!m_bIsPlayable) return;
		if (m_AIAgent)
			m_AIAgent.DeactivateAI();

		GetGame().GetCallqueue().CallLater(AddToListWrap, 0, false, owner) // init delay
	}

	private void AddToListWrap(IEntity owner)
	{
		if (!m_bIsPlayable) return;
		if (m_AIAgent)
			m_AIAgent.DeactivateAI();

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		rpl.EnableStreaming(false);

		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();

		m_id = rpl.Id();
		playableManager.RegisterPlayable(this);
	}

	string GetName()
	{
		if (m_name != "") return m_name;
		return m_EditableUIInfo.GetName();
	}

	RplId GetId()
	{
		return m_id;
	}

	void PS_PlayableComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		
	}

	void ~PS_PlayableComponent()
	{
		RemoveFromList();
	}

	// Send our precision data, we need it on clients
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteString(m_name);
		writer.WriteBool(m_bIsPlayable);
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadString(m_name);
		reader.ReadBool(m_bIsPlayable);
		return true;
	}
}
