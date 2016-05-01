using UnityEngine;
using System.Collections;
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
    public static extern void GameTick();
    [DllImport("THShared")]
    public static extern void GameCreateRoom(byte playersCount, RoomCreationCallback callback);
    [DllImport("THShared")]
    public static extern void GameJoinRoom(uint roomId, JoinRoomCallback callback);
    [DllImport("THShared")]
    public static extern void GameStart(StartGameCallback callback);
    [DllImport("THShared")]
    public static extern void GameSendInput(float x, float y);
    [DllImport("THShared")]
    public static extern void GameReceivePosition(out float x, out float y);
    [DllImport("THShared")]
    public static extern void GamePause();
    [DllImport("THShared")]
    public static extern void GameResume();
    [DllImport("THShared")]
    public static extern void GameQuit();

    public Transform cube;
    protected bool paused = false;

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
        bool isPlaying = false;
        int gameState = GameGetState();
        if (gameState > 0)
        {
            if (Input.GetKeyDown(KeyCode.S))
                Debug.Log("RTT: " + GameGetRTT());
        }
        switch (gameState)
        {
            case 0: // disconnected
                break;
            case 1: // connected
                if (Input.GetKeyDown(KeyCode.A))
                    GameCreateRoom(1, (roomId) =>
                    {
                        Debug.Log("CreateRoom said " + roomId);
                        if (roomId != 0xffffffff)
                        {
                            GameJoinRoom(roomId, (success) =>
                            {
                                Debug.Log("JoinRoom said " + success);
                            });
                        }
                    });
                break;
            case 2: // joined room
                if (Input.GetKeyDown(KeyCode.A))
                {
                    Debug.Log("Starting game...");
                    GameStart((success) =>
                    {
                        Debug.Log("StartGame said " + success);
                    });
                }
                break;
            case 3: // waiting
                break;
            case 4: // playing
                isPlaying = true;
                break;
        }

        if (isPlaying)
        {
            GameSendInput(Input.GetAxis("Horizontal"), Input.GetAxis("Vertical"));

            GameTick();

            float x, y;
            GameReceivePosition(out x, out y);
            cube.position = new Vector3(x, .0f, y);
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
}
