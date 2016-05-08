using System;
using System.Collections.Generic;
using UnityEngine;

public class Player : MonoBehaviour
{
    public enum State
    {
        Idle = 0,
        Moving,
        Attacking
    }

    Vector3 prevPos;
    float prevTime;

    Vector3 instantVel;
    Vector3 vel;

    Vector3 prevDir;

    Animation anim;

    void Awake()
    {
        prevPos = transform.position;
        prevDir = transform.forward;
        prevTime = Time.unscaledTime;

        anim = gameObject.GetComponent<Animation>();
    }

    public void SetState(float x, float y, float dx, float dy, State state, float time)
    {
        float newTime = Time.unscaledTime, dt = newTime - prevTime;
        Vector3 newPos = new Vector3(x, .0f, y);

        instantVel = (newPos - prevPos) / (newTime - prevTime);
        vel = Vector3.RotateTowards(vel, instantVel, Mathf.PI * 3.0f * dt, 6.0f * dt);

        prevTime = newTime;
        prevPos = newPos;

        Vector3 newDir = new Vector3(dx, 0.0f, dy).normalized;
        prevDir = Vector3.RotateTowards(prevDir, newDir, Mathf.PI * 3.0f * dt, 0.0f);

        transform.position = newPos;
        transform.rotation = Quaternion.LookRotation(prevDir, Vector3.up);
        //Debug.Log("state: " + state + ", time " + time);
        switch (state)
        {
            case State.Idle:
                anim.CrossFade("Wait", 0.25f);
                break;
            case State.Moving:
                anim.CrossFade("Walk", 0.25f);
                anim["Walk"].speed = vel.magnitude * 0.25f;
                break;
            case State.Attacking:
                anim.Play("Attack", PlayMode.StopAll);
                anim["Attack"].speed = 0.0f;
                anim["Attack"].normalizedTime = time / 0.24f;
                break;
        }
    }
}
