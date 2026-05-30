// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Minimal high-resolution stopwatch for measuring elapsed time.
 * Wraps FPlatformTime::Seconds() (monotonic, frame-accurate — PRD §6.3).
 * Plain C++ struct — not a UObject; just hold one as a member.
 *
 *   FStopwatch SW;
 *   SW.Start();                 // e.g. at stimulus onset
 *   ...
 *   const double Rt = SW.ElapsedMs();   // e.g. on click
 */
struct FStopwatch
{
	// Begin (or restart) timing from now.
	void Start()
	{
		StartSeconds = FPlatformTime::Seconds();
		StopSeconds = StartSeconds;
		bRunning = true;
	}

	// Freeze the elapsed time at this moment. Optional — ElapsedX() works while running.
	void Stop()
	{
		if (bRunning)
		{
			StopSeconds = FPlatformTime::Seconds();
			bRunning = false;
		}
	}

	// Clear back to zero.
	void Reset()
	{
		StartSeconds = 0.0;
		StopSeconds = 0.0;
		bRunning = false;
	}

	// Time since Start(), in seconds. Live while running, frozen after Stop().
	double ElapsedSeconds() const
	{
		const double End = bRunning ? FPlatformTime::Seconds() : StopSeconds;
		return End - StartSeconds;
	}

	// Same, in milliseconds.
	double ElapsedMs() const
	{
		return ElapsedSeconds() * 1000.0;
	}

	bool IsRunning() const { return bRunning; }

private:
	double StartSeconds = 0.0;
	double StopSeconds = 0.0;
	bool bRunning = false;
};
