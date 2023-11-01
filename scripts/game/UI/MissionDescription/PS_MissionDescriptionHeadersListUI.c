

class PS_MissionDescriptionHeadersListUI : ScriptedWidgetComponent
{
	protected ResourceName m_rHeaderLayout = "{724F2A3021D3BFAD}UI/MissionDescription/MissionDescriptionSelector.layout";
	
	protected VerticalLayoutWidget m_vMissionDescriptionList;
	protected Widget m_wRoot;
	
	override void HandlerAttached(Widget w)
	{
		m_vMissionDescriptionList = VerticalLayoutWidget.Cast(w.FindAnyWidget("MissionDescriptionList"));
		if (!GetGame().InPlayMode())
			return;
		m_wRoot = w;
		GetGame().GetCallqueue().CallLater(FillList, false, 0);
	}
	
	void FillList()
	{
		PS_PlayableManager playableManager = PS_PlayableManager.GetInstance();
		
		PlayerController currentPlayerController = GetGame().GetPlayerController();
		int currentPlayerId = currentPlayerController.GetPlayerId();
		FactionKey factionKey = playableManager.GetPlayerFactionKey(currentPlayerId);
		
		PS_MissionDescriptionManager missionDescriptionManager = PS_MissionDescriptionManager.GetInstance();
		array<PS_MapDescription> descriptions = new array<PS_MapDescription>;
		missionDescriptionManager.GetDescriptions(descriptions);
		
		foreach (PS_MapDescription description: descriptions)
		{
			if (description.m_aFactions.Count() > 0)
				if (!description.m_aFactions.Contains(factionKey))
					continue;
			
			Widget header = GetGame().GetWorkspace().CreateWidgets(m_rHeaderLayout);
			PS_MissionDescriptionSelectorUI handler = PS_MissionDescriptionSelectorUI.Cast(header.FindHandler(PS_MissionDescriptionSelectorUI));
			handler.SetTitle(description.m_sTitle);
			handler.SetContent(description.m_sDescriptionLayout);
			
			PS_MissionDescriptionContentUI handlerContent = PS_MissionDescriptionContentUI.Cast(header.FindHandler(PS_MissionDescriptionContentUI));
			PS_MissionDescriptionContentUI currentHandlerContent = PS_MissionDescriptionContentUI.Cast(m_wRoot.FindHandler(PS_MissionDescriptionContentUI));
			handlerContent.SetMissionUI(currentHandlerContent.GetMissionUI());
			
			m_vMissionDescriptionList.AddChild(header);
		}
	}
}