using System;
using System.Runtime.InteropServices;
using UnityEngine;

public class PluginImport : MonoBehaviour
{
    //Lets make our calls from the Plugin
    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingSync(string ip_addr);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern uint PingAsync(string ip_addr);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern bool PingIsDone(uint ip_handle);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingTime(uint ip_handle);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingStatus(uint ip_handle);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern void SetLogLevel(uint log_level);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern void SetTimeout(uint timeout);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern void SetNumIterations(uint num_iterations);

    void Start()
    {
        SetNumIterations(1);
        Debug.Log("pinging 127.0.0.1");
        Debug.Log(PingSync("127.0.0.1"));
        Debug.Log("pinging 192.168.0.1");
        Debug.Log(PingSync("192.168.0.1"));
        Debug.Log("pinging www.unity.com");
        Debug.Log(PingSync("www.unity.com"));

        Debug.Log("pinging www.unity.com asynchronously:");
        int handle = PingAync("www.unity.com");
        while (!PingIsDone(handle))
        {
            Sleep(1);
        }
        Debug.Log("status:");
        Debug.Log(PingStatus(handle));
        Debug.Log("time:");
        Debug.Log(PingTime(handle));
    }
}
