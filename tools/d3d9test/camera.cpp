#include <cstdio>
#include <cassert>
#include <cmath>

#define RW_D3D9
#include <rw.h>

#define PI 3.14159265359f
#include "camera.h"

using rw::Quat;
using rw::V3d;

void
Camera::update(void)
{
	if(m_rwcam){
		m_rwcam->nearPlane = m_near;
		m_rwcam->farPlane = m_far;
		m_rwcam->setFOV(m_fov, m_aspectRatio);
		m_rwcam->updateProjectionMatrix();

		V3d forward = m_target.sub(m_position).normalize();
		V3d left = m_up.cross(forward).normalize();
		V3d nup = forward.cross(left);

		rw::Frame *f = m_rwcam->getFrame();
		f->matrix.right = left; // lol
		f->matrix.up = nup;
		f->matrix.at = forward;
		f->matrix.pos = m_position;
		f->updateObjects();
	}
}

void
Camera::setTarget(V3d target)
{
	m_position = m_position.sub(m_target.sub(target));
	m_target = target;
}

float
Camera::getHeading(void)
{
	V3d dir = m_target.sub(m_position);
	float a = atan2(dir.y, dir.x)-PI/2.0f;
	return m_local_up.z < 0.0f ? a-PI : a;
}

void
Camera::turn(float yaw, float pitch)
{
	yaw /= 2.0f;
	pitch /= 2.0f;
	V3d dir = m_target.sub(m_position);
	Quat r(cos(yaw), 0.0f, 0.0f, sin(yaw));
	dir = r.mult(Quat(dir).mult(r.conj())).vec();
	m_local_up = r.mult(Quat(m_local_up).mult(r.conj())).vec();

	V3d right = dir.cross(m_local_up).normalize();
	r = Quat(cos(pitch), right.scale(sin(pitch)));
	dir = r.mult(Quat(dir).mult(r.conj())).vec();
	m_local_up = right.cross(dir).normalize();
	if(m_local_up.z >= 0.0) m_up.z = 1.0;
	else m_up.z = -1.0f;

	m_target = m_position.add(dir);
}

void
Camera::orbit(float yaw, float pitch)
{
	yaw /= 2.0f;
	pitch /= 2.0f;
	V3d dir = m_target.sub(m_position);
	Quat r(cos(yaw), 0.0f, 0.0f, sin(yaw));
	dir = r.mult(Quat(dir).mult(r.conj())).vec();
	m_local_up = r.mult(Quat(m_local_up).mult(r.conj())).vec();

	V3d right = dir.cross(m_local_up).normalize();
	r = Quat(cos(-pitch), right.scale(sin(-pitch)));
	dir = r.mult(Quat(dir).mult(r.conj())).vec();
	m_local_up = right.cross(dir).normalize();
	if(m_local_up.z >= 0.0) m_up.z = 1.0;
	else m_up.z = -1.0f;

	m_position = m_target.sub(dir);
}

void
Camera::dolly(float dist)
{
	V3d dir = m_target.sub(m_position).normalize().scale(dist);
	m_position = m_position.add(dir);
	m_target = m_target.add(dir);
}

void
Camera::zoom(float dist)
{
	V3d dir = m_target.sub(m_position);
	float curdist = dir.length();
	if(dist >= curdist)
		dist = curdist-0.01f;
	dir = dir.normalize().scale(dist);
	m_position = m_position.add(dir);
}

void
Camera::pan(float x, float y)
{
	V3d dir = m_target.sub(m_position).normalize();
	V3d right = dir.cross(m_up).normalize();
	V3d local_up = right.cross(dir).normalize();
	dir = right.scale(x).add(local_up.scale(y));
	m_position = m_position.add(dir);
	m_target = m_target.add(dir);
}

float
Camera::distanceTo(V3d v)
{
	return m_position.sub(v).length();
}

Camera::Camera()
{
	m_position.set(0.0f, 6.0f, 0.0f);
	m_target.set(0.0f, 0.0f, 0.0f);
	m_up.set(0.0f, 0.0f, 1.0f);
	m_local_up = m_up;
	m_fov = 70.0f;
	m_aspectRatio = 1.0f;
	m_near = 0.1f;
	m_far = 100.0f;
	m_rwcam = NULL;
}

