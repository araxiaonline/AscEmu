/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Utilities/Util.hpp"
#include "Movement/MovementGenerator.h"
#include "Movement/Spline/MoveSplineInit.h"

class Unit;

enum MovementGeneratorType : uint8_t;

class GenericMovementGenerator : public MovementGenerator
{
public:
    explicit GenericMovementGenerator(MovementMgr::MoveSplineInit&& splineInit, MovementGeneratorType type, uint32_t id);

    void initialize(Unit*) override;
    void reset(Unit*) override;
    bool update(Unit*, uint32_t) override;
    void deactivate(Unit*) override;
    void finalize(Unit*, bool, bool) override;
    MovementGeneratorType getMovementGeneratorType() const override { return _type; }

private:
    void movementInform(Unit*);

    MovementMgr::MoveSplineInit _splineInit;
    MovementGeneratorType _type;
    uint32_t _pointId;
    SmallTimeTracker _duration;
};
