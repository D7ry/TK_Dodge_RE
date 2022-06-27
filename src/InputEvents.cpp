#include "InputEvents.h"
#include "DataHandler.h"
#include "AnimationEvents.h"

std::uint32_t InputEventHandler::GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key)
{
	using Key = RE::BSWin32GamepadDevice::Key;

	std::uint32_t index;
	switch (a_key) {
	case Key::kUp:
		index = 0;
		break;
	case Key::kDown:
		index = 1;
		break;
	case Key::kLeft:
		index = 2;
		break;
	case Key::kRight:
		index = 3;
		break;
	case Key::kStart:
		index = 4;
		break;
	case Key::kBack:
		index = 5;
		break;
	case Key::kLeftThumb:
		index = 6;
		break;
	case Key::kRightThumb:
		index = 7;
		break;
	case Key::kLeftShoulder:
		index = 8;
		break;
	case Key::kRightShoulder:
		index = 9;
		break;
	case Key::kA:
		index = 10;
		break;
	case Key::kB:
		index = 11;
		break;
	case Key::kX:
		index = 12;
		break;
	case Key::kY:
		index = 13;
		break;
	case Key::kLeftTrigger:
		index = 14;
		break;
	case Key::kRightTrigger:
		index = 15;
		break;
	default:
		index = kInvalid;
		break;
	}

	return index != kInvalid ? index + kGamepadOffset : kInvalid;
}

void InputEventHandler::offsetButtonEventID(RE::ButtonEvent* a_event, uint32_t& id)
{
	using DeviceType = RE::INPUT_DEVICE;
	switch (a_event->device.get()) {
	case DeviceType::kMouse:
		id += kMouseOffset;
		break;
	case DeviceType::kKeyboard:
		id += kKeyboardOffset;
		break;
	case DeviceType::kGamepad:
		id = GetGamepadIndex(static_cast<RE::BSWin32GamepadDevice::Key>(id));
		break;
	}
}

inline uint32_t InputEventHandler::getOffsetButtonIDCode(RE::ButtonEvent* a_event)
{
	auto id = a_event->idCode;
	using DeviceType = RE::INPUT_DEVICE;
	switch (a_event->device.get()) {
	case DeviceType::kMouse:
		id += kMouseOffset;
		break;
	case DeviceType::kKeyboard:
		id += kKeyboardOffset;
		break;
	case DeviceType::kGamepad:
		id = GetGamepadIndex(static_cast<RE::BSWin32GamepadDevice::Key>(id));
		break;
	}
	return id;
}



EventResult InputEventHandler::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
{
	using EventType = RE::INPUT_EVENT_TYPE;
	
	if (!a_event) 
		return EventResult::kContinue;

	const auto ui = RE::UI::GetSingleton();
	if (ui->GameIsPaused()) {
		return EventResult::kContinue;
	}
	const auto controlMap = RE::ControlMap::GetSingleton();
	if (!controlMap->IsMovementControlsEnabled() || !controlMap->IsFightingControlsEnabled()) {
		return EventResult::kContinue;
	}

	auto datahandler = DataHandler::GetSingleton();


	for (auto event = *a_event; event; event = event->next) {
		if (event->eventType != EventType::kButton)
			continue;

		const auto button = static_cast<RE::ButtonEvent*>(event);
		if (!button || !button->IsDown()) {
			continue;
		}

		if (!button->HasIDCode()) {
			continue;
		}

		uint32_t id = getOffsetButtonIDCode(button);

		if (id == Settings::dodgeKey) {
			dodge();
		}

	}

	return EventResult::kContinue;
}
inline bool isJumping(RE::Actor* a_actor) {
	bool result = false;
	return a_actor->GetGraphVariableBool("bInJumpState", result) && result;
}

void InputEventHandler::dodge() {

	logger::info("dodging");
	auto pc = RE::PlayerCharacter::GetSingleton();
	if (pc->IsSprinting() && Settings::enableTappingSprint) {
		return;
	}
	const std::string dodge_event = DataHandler::GetSingleton()->GetDodgeEvent();
	if (!dodge_event.empty() && pc->GetSitSleepState() == RE::SIT_SLEEP_STATE::kNormal && pc->GetKnockState() == RE::KNOCK_STATE_ENUM::kNormal &&
		pc->GetFlyState() == RE::FLY_STATE::kNone && (!pc->IsSneaking() || Settings::enableSneakDodge) && !pc->IsSwimming() &&
		!isJumping(pc) && !pc->IsInKillMove() && (pc->GetActorValue(RE::ActorValue::kStamina) >= Settings::dodgeStamina)) {
		//DEBUG(FMT_STRING("{} Trigger!"), dodge_event);
		bool IsDodging = false;
		if (pc->GetGraphVariableBool("bIsDodging", IsDodging) && IsDodging) {
			//DEBUG("Player is already dodging!");
			return;
		}
		if (Settings::stepDodge) {
			pc->SetGraphVariableInt("iStep", 2);
		}
		pc->SetGraphVariableFloat("TKDR_IframeDuration", Settings::iFrameDuration);  //Set invulnerable frame duration
		pc->NotifyAnimationGraph(dodge_event);                                                    //Send TK Dodge Event
	}
}





