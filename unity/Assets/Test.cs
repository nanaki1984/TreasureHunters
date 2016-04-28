using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;
using System;

public class Test : MonoBehaviour {
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LogCallback(int type, string str);

    [DllImport("THShared")]
    public static extern bool GameInit(string serverHost, int serverPort, LogCallback debugLog);
    [DllImport("THShared")]
    public static extern void GameTick();
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

    void Awake()
    {
        GameInit("localhost", 1234, (type, str) =>
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
	}

    float t = 0.0f;
	void Update()
    {
        GameTick();

        if (t <= 0.0f)
        {
            GameSendInput(Input.GetAxis("Horizontal"), Input.GetAxis("Vertical"));
            t += 0.033f;
        }

        t -= Time.unscaledDeltaTime;

        float x, y;
        GameReceivePosition(out x, out y);
        cube.position = new Vector3(x, .0f, y);
    }

    void OnApplicationPause(bool paused)
    {
        if (paused)
            GamePause();
        else
            GameResume();
    }

    void OnApplicationQuit()
    {
        GameQuit();
    }
}
