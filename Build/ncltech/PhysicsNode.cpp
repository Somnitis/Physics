#include "PhysicsNode.h"
#include "PhysicsEngine.h"


void PhysicsNode::IntegrateForVelocity(float dt)
{
	// Apply gravity
	if (invMass > 0.0f) {
		linVelocity += PhysicsEngine::Instance()->GetGravity()*dt;
	}

	// Euler Semi Intergration
	linVelocity += force * invMass *dt;
	// Velocity Damping
	linVelocity = linVelocity * PhysicsEngine::Instance()->GetDampingFactor();

	// Angular Rotation
	angVelocity += invInertia * torque *dt;
	// Angular Rotation Damping
	angVelocity = angVelocity * PhysicsEngine::Instance()->GetDampingFactor();
}

/* Between these two functions the physics engine will solve for velocity
based on collisions/constraints etc. So we need to integrate velocity, solve
constraints, then use final velocity to update position.
*/

void PhysicsNode::IntegrateForPosition(float dt)
{
	// Update Position
	position += linVelocity *dt;

	// Update Orientation
	orientation = orientation + Quaternion(angVelocity * dt * 0.5f, 0.0f) * orientation;

	invInertia = invInertia * (Quaternion(angVelocity * dt*0.5f, 0.f)*orientation).ToMatrix3();

	orientation.Normalise();

	//Finally: Notify any listener's that this PhysicsNode has a new world transform.
	// - This is used by GameObject to set the worldTransform of any RenderNode's. 
	//   Please don't delete this!!!!!
	FireOnUpdateCallback();
}