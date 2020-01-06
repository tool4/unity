using System;
using System.Runtime.InteropServices;
using UnityEngine;

public class PluginImport : MonoBehaviour
{
    //Lets make our calls from the Plugin
    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingSync(string ip_addr);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingAsync(string ip_addr);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingIsDone(int ip_handle);

    [DllImport("PingPlugin", CallingConvention = CallingConvention.Cdecl)]
    private static extern int PingTime(int ip_handle);

    void Start()
    {
        Debug.Log(PingSync("127.0.0.1"));
        Debug.Log(PingSync("192.168.0.1"));
        Debug.Log(PingSync("www.unity.com"));
    }
}
