using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;

public class Test : MonoBehaviour {
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LogCallback(int type, string str);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void RoomCreationCallback(uint roomId);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void JoinRoomCallback(bool success);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void StartGameCallback(bool success);

    [DllImport("THShared")]
    public static extern void GameLoadLibrary();
    [DllImport("THShared")]
    public static extern void GameUnloadLibrary();
    [DllImport("THShared")]
    public static extern bool GameInit(string serverHost, int serverPort, LogCallback debugLog);
    [DllImport("THShared")]
    public static extern int GameGetState();
    [DllImport("THShared")]
    public static extern float GameGetRTT();
    [DllImport("THShared")]
    public static extern byte GameGetRoomId();
    [DllImport("THShared")]
    public static extern void GameTick();
    [DllImport("THShared")]
    public static extern void GameCreateRoom(byte playersCount, RoomCreationCallback callback);
    [DllImport("THShared")]
    public static extern void GameJoinRoom(uint roomId, JoinRoomCallback callback);
    [DllImport("THShared")]
    public static extern void GameStart(StartGameCallback callback);
    [DllImport("THShared")]
    public static extern void GameSendInput(float x, float y, bool attack);
    [DllImport("THShared")]
    public static extern byte GameGetPlayersCount();
    [DllImport("THShared")]
    public static extern void GameReceiveState(byte id, out float x, out float y, out float dx, out float dy, out int state, out float time);
    [DllImport("THShared")]
    public static extern void GameReceiveEnemyPosition(out float x, out float y);
    [DllImport("THShared")]
    public static extern void GamePause();
    [DllImport("THShared")]
    public static extern void GameResume();
    [DllImport("THShared")]
    public static extern void GameQuit();

    public Player playerPrefab;
    public Transform enemy;

    protected bool paused = false;

    protected List<Player> players = new List<Player>();

    void Awake()
    {
        GameLoadLibrary();

        var success = GameInit("localhost", 1234, (type, str) =>
        {
            switch (type)
            {
                case 0:
                    Debug.Log(str);
                    break;
                case 1:
                    Debug.LogWarning(str);
                    break;
                case 2:
                    Debug.LogError(str);
                    break;
            }
        });
        Debug.Log("GameInit said " + success);
	}

    void Update()
    {
        int gameState = GameGetState();
        bool isPlaying = (4 == gameState);
        if (gameState > 0)
        {
            if (Input.GetKeyDown(KeyCode.S))
                Debug.Log("RTT: " + GameGetRTT());
        }

        if (isPlaying)
        {
            GameSendInput(Input.GetAxis("Horizontal"), Input.GetAxis("Vertical"), Input.GetKey(KeyCode.X));

            GameTick();

            float x, y, dx, dy, t;
            int state;

            for (int i = 0, c = players.Count; i < c; ++i)
            {
                GameReceiveState((byte)i, out x, out y, out dx, out dy, out state, out t);
                players[i].SetState(x, y, dx, dy, (Player.State)state, t);
            }

            GameReceiveEnemyPosition(out x, out y);
            enemy.position = new Vector3(x, .0f, y);
        }
        else
            GameTick();
    }

    void OnApplicationPause(bool flag)
    {
        if (flag)
        {
            if (!paused)
            {
                Debug.LogWarning("GamePaused");
                GamePause();
                paused = true;
            }
        }
        else
        {
            if (paused)
            {
                Debug.LogWarning("GameResumed");
                GameResume();
                paused = false;
            }
        }
    }

    void OnApplicationQuit()
    {
        GameQuit();
#if !UNITY_EDITOR
        GameUnloadLibrary();
#endif
    }

    int playersCount = 1;
    int joinRoomId = 0;
    void OnGUI()
    {
        int gameState = GameGetState();
        GUILayout.BeginVertical();
        switch (gameState)
        {
            case 1:
                GUILayout.BeginHorizontal(GUILayout.Width(400));
                GUILayout.Label("Players count: ");
                playersCount = int.Parse(GUILayout.TextField(playersCount.ToString()));
                if (GUILayout.Button("Create and join room"))
                    GameCreateRoom((byte)playersCount, (roomId) =>
                    {
                        Debug.Log("CreateRoom said " + roomId);
                        if (roomId != 0xffffffff)
                        {
                            GameJoinRoom(roomId, (success) =>
                            {
                                Debug.Log("JoinRoom said " + success);
                                for (int i = players.Count - 1; i >= 0; ++i)
                                {
                                    GameObject.Destroy(players[i].gameObject);
                                    players.RemoveAt(i);
                                }
                                for (int i = 0, c = GameGetPlayersCount(); i < c; ++i)
                                    players.Add(Player.Instantiate(playerPrefab));
                            });
                        }
                    });
                GUILayout.EndHorizontal();
                GUILayout.BeginHorizontal();
                GUILayout.Label("Join room id: ");
                joinRoomId = int.Parse(GUILayout.TextField(joinRoomId.ToString()));
                if (GUILayout.Button("Join room"))
                    GameJoinRoom((uint)joinRoomId, (success) =>
                    {
                        Debug.Log("JoinRoom said " + success);
                        for (int i = players.Count - 1; i >= 0; ++i)
                        {
                            GameObject.Destroy(players[i].gameObject);
                            players.RemoveAt(i);
                        }
                        for (int i = 0, c = GameGetPlayersCount(); i < c; ++i)
                            players.Add(Player.Instantiate(playerPrefab));
                    });
                GUILayout.EndHorizontal();
                break;
            case 2:
                GUILayout.BeginHorizontal(GUILayout.Width(400));
                GUILayout.Label("Joined room id: " + GameGetRoomId());
                if (GUILayout.Button("Start game"))
                    GameStart((success) =>
                    {
                        Debug.Log("StartGame said " + success);
                    });
                GUILayout.EndHorizontal();
                // ToDo: leave room/disconnect
                break;
        }
        GUILayout.EndVertical();
    }
}
