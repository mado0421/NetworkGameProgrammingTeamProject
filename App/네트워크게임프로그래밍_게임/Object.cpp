#include "stdafx.h"
#include "Object.h"


Object::Object()
{
}

Object::~Object()
{
}

void Object::update(float elapsedTime)
{
	move(elapsedTime);
}

void Object::render() const
{
	glPushMatrix();
	glColor3f(m_color.r, m_color.g, m_color.b);
	glTranslatef(m_pos.x, m_pos.y, 0.0f);

	glutSolidCube(m_size * 2.0);

	glPopMatrix();
}

bool Object::isOut()
{
	return(m_pos.x - m_size < 0 || m_pos.x + m_size > WWIDTH ||
		m_pos.y - m_size < 0 || m_pos.y + m_size > WHEIGHT);
}

bool Object::isDead()
{
	return (m_hp <= 0);
}

bool Object::isCollide(const Object & other)
{
	return (
		(m_size + other.m_size)*(m_size + other.m_size) >=
		(m_pos.x + m_dir.x - other.m_pos.x)*(m_pos.x + m_dir.x - other.m_pos.x) +
		(m_pos.y + m_dir.y - other.m_pos.y)*(m_pos.y + m_dir.y - other.m_pos.y));
}

bool Object::isCollideRect(const Object & other)
{
	return (
		m_pos.x + m_size + m_dir.x > other.m_pos.x - other.m_size &&
		m_pos.x - m_size + m_dir.x < other.m_pos.x + other.m_size &&
		m_pos.y + m_size + m_dir.y > other.m_pos.y - other.m_size &&
		m_pos.y - m_size + m_dir.y < other.m_pos.y + other.m_size);
}

void Object::move(float val)
{
}

//////////////////////////////////////////////////////////////////////////

Player::Player()
{
}

Player::~Player()
{
}

void Player::changeMoveDirection(unsigned char dir, float val)
{
	switch (dir)
	{
	case 'x':
		m_dirX = val;
		break;
	case 'y':
		m_dirY = val;
		break;
	default:
		break;
	}
	m_dir = Vector::normalize(Vector2D(m_dirX, m_dirY));
}

int Player::getMoveDirection(unsigned char dir)
{
	switch (dir)
	{
	case 'x':
		return m_dirX;
	case 'y':
		return m_dirY;
	default:
		break;
	}
	m_dir = Vector::normalize(Vector2D(m_dirX, m_dirY));
}

void Player::update(float elapsedTime)
{
	move(elapsedTime);
}

void Player::move(float val)
{
	m_pos.x += val * m_spd * m_dir.x;
	m_pos.y += val * m_spd * m_dir.y;
}

void Player::changeDirByCollide(const Object & other)
{
	Vector2D pos = other.getPos();
	if ((pos.x - m_pos.x) * (pos.x - m_pos.x) > (pos.y - m_pos.y) * (pos.y - m_pos.y)) m_dir.x = 0.0f;
	else m_dir.y = 0.0f;
}

void Player::fire()
{
	m_ammo--;
}

void Player::reload()
{
	m_ammo = MAX_AMMO;
}

void Player::dash()
{
}

//////////////////////////////////////////////////////////////////////////

Bullet::Bullet()
{
}

Bullet::~Bullet()
{
}

void Bullet::move(float val)
{
	m_pos.x += val * m_spd * m_dir.x;
	m_pos.y += val * m_spd * m_dir.y;
}

//////////////////////////////////////////////////////////////////////////

Item::Item()
{
}

Item::~Item()
{
}

Tile::Tile()
{
}

Tile::~Tile()
{
}

void Tile::render() const
{
	if (type == tile::Wall)
	{
		glPushMatrix();
		glColor3f(m_color.r, m_color.g, m_color.b);
		glTranslatef(m_pos.x, m_pos.y, 0.0f);

		glutSolidCube(m_size * 2.0);

		glPopMatrix();
	}
}
