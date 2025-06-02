using System;
using Unity.Collections;
using UnityEngine;

/** 
 * Tracks how many things were created / moved etc during each playphase as well as overall
 * Also creates quests for those things tracked
 */
public class Statistics : SaveableService
{
    public int BestHighscore = 0;

    public int GetHighscore()
    {
        int Highscore = 0;

        if (Highscore > BestHighscore)
        {
            BestHighscore = Highscore;
        }
        return Highscore;
    }

    public void IncreaseTarget(ref int Goal, int GoalIncrease)
    {
        Goal += GoalIncrease;
    }


    /*
    public void CountMoves(DiscoveryState State)
    {
        if (State < DiscoveryState.Visited)
            return;

        MovesDone += 1;
        CurrentMoves += 1;
        BestMoves = Math.Max(CurrentMoves, BestMoves);
    }
    */

    private void Subscribe(bool bSubscribe)
    {
        if (bSubscribe)
        {
            //BuildingService._OnBuildingBuilt.Add(CountBuilding);
        }
        else
        {
           // BuildingService._OnBuildingBuilt.Remove(CountBuilding);
        }
    }
    protected override void StartServiceInternal()
    {
        Game.RunAfterServiceInit((SaveGameManager Manager) =>
        {
            Subscribe(true);
            if (!Manager.HasDataFor(SaveableService.SaveGameType.Statistics))
            {
                ResetAllStats();
            }
            _OnInit?.Invoke(this);
        });
    }

    protected override void ResetInternal()
    {
        ResetAllStats();
        Subscribe(false);
    }

    public override void OnAfterLoaded()
    {
        base.OnAfterLoaded();
        Subscribe(true);
        _OnInit?.Invoke(this);
    }

    private void ResetAllStats()
    {
    }

    private void ResetCurrentStats()
    {
    }

    protected override void StopServiceInternal() {
        Subscribe(false);
    }

    public override void OnBeforeSaved(bool bShouldReset)
    {
        ResetCurrentStats();
    }

    public GameObject GetGameObject() { return gameObject; }

}
