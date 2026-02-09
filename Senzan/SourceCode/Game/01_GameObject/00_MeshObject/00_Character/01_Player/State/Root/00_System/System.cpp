#include "System.h"

#include "../../Constant.h"

namespace PlayerState {

System::System(Player* owner)
	: PlayerStateBase( owner )
{

}
System::~System()
{
}
void System::Enter()
{
}
void System::Update()
{
}
void System::LateUpdate()
{
}
void System::Draw()
{
}
void System::Exit()
{
}
} // PlayerState.
