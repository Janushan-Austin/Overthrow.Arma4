class OVT_PlayerIdentityComponentClass: OVT_ComponentClass
{
};

class OVT_PlayerIdentityComponent: OVT_Component
{		
	SCR_CharacterControllerComponent m_Controller;
	protected string m_sPersistentID = "";
	
	protected const string PERSISTENT_ID_FILE_PATH = "$profile:overthrowPersistentID.txt";
	
	static ref map<int, string> m_mPersistentIDs;
	static ref map<string, int> m_mPlayerIDs;
	
	static string GetPersistentIDFromPlayerID(int playerId)
	{
		return m_mPersistentIDs[playerId];
	}
	
	static int GetPlayerIDFromPersistentID(string id)
	{
		return m_mPlayerIDs[id];
	}
	
	string GetPersistentID()
	{
		return m_sPersistentID;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);		
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast( owner.FindComponent(SCR_CharacterControllerComponent) );		
				
		if(controller){
			m_Controller = controller;			
			m_Controller.m_OnControlledByPlayer.Insert(this.OnControlledByPlayer);
		}
	}
	
	protected void OnControlledByPlayer(IEntity owner, bool controlled)
	{	
		if(m_sPersistentID != "") return;	
		if (controlled)
		{			
			//Check for a saved persistent player ID			
			if(FileIO.FileExist(PERSISTENT_ID_FILE_PATH))
			{
				//File exists, use it
				FileHandle f = FileIO.OpenFile(PERSISTENT_ID_FILE_PATH, FileMode.READ);
				if(f){
					f.FGets(m_sPersistentID);
					f.CloseFile();
				}
			}else{
				//File doesnt exist, generate one
				m_sPersistentID = GenerateID(); //To-Do: Generate a random string
				FileHandle f = FileIO.OpenFile(PERSISTENT_ID_FILE_PATH, FileMode.WRITE);
				f.FPrint(m_sPersistentID);
				f.CloseFile();
			}
			Rpc(RpcAsk_SetID, m_sPersistentID);
		}
	}
	
	protected string GenerateID()
	{
			
		int year = 0;
		int month = 0;
		int day = 0;	
		System.GetYearMonthDayUTC(year, month, day);
	
		int hour = 0;
		int minute = 0;
		int second = 0;			
		System.GetHourMinuteSecondUTC(hour, minute, second);
		
		string s = ""+year+month+day+hour+minute+second;
	
		//Add some random numbers (Reference: https://eager.io/blog/how-long-does-an-id-need-to-be/)
		int i = s_AIRandomGenerator.RandFloatXY(0, 16777215);
		s += i.ToString();
	
		i = s_AIRandomGenerator.RandFloatXY(0, 16777215);
		s += i.ToString();
		
		return s;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SetID(string id)
	{
		m_sPersistentID = id;
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner());
		m_mPersistentIDs[playerId] = id;
		m_mPlayerIDs[id] = playerId;
		
		//Broadcast this to all players
		Rpc(RpcDo_SetID, playerId, id);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetID(int playerId, string id)
	{
		m_mPersistentIDs[playerId] = id;
		m_mPlayerIDs[id] = playerId;
	}
	
	void OVT_PlayerIdentityComponent()
	{
		m_mPersistentIDs = new map<int, string>;
		m_mPlayerIDs = new map<string, int>;
	}
	
	void ~OVT_PlayerIdentityComponent()
	{		
		if(!m_Controller) return;
		
		m_Controller.m_OnControlledByPlayer.Remove(this.OnControlledByPlayer);
	}
}