using System;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    Vector3 prevPos;
    Vector3 prevDir;
    float prevTime;

    Vector3 instantVel;
    Vector3 vel;

    void Awake()
    {
        prevPos = transform.position;
        prevDir = transform.forward;
        prevTime = Time.unscaledTime;
    }

    public void SetPosition(float x, float y)
    {
        float newTime = Time.unscaledTime, dt = newTime - prevTime;
        Vector3 newPos = new Vector3(x, .0f, y);

        instantVel = (newPos - prevPos) / (newTime - prevTime);
        vel = Vector3.RotateTowards(vel, instantVel, Mathf.PI * 3.0f * dt, 6.0f * dt);

        prevTime = newTime;
        prevPos = newPos;

        transform.position = newPos;
        if (Vector3.SqrMagnitude(vel) > 0.01f)
            transform.rotation = Quaternion.LookRotation(vel, Vector3.up);
    }
}
