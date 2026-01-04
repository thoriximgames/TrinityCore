USE `world`;

-- 1. Essential Trinity Strings (Version and generic messages)
DELETE FROM `trinity_string` WHERE `entry` < 100;
INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES 
(1, 'TrinityCore/3.3.5a (Branch: 3.3.5)'),
(2, 'Using database: TDB 335.25101'),
(3, 'World initialized'),
(4, 'Account created: %s');

-- 2. Command Table (So you can actually use the console)
DELETE FROM `command`;
INSERT INTO `command` (`name`, `help`) VALUES 
('account', 'Syntax: .account ...'),
('account create', 'Syntax: .account create <name> <pass>'),
('account set', 'Syntax: .account set ...'),
('server', 'Syntax: .server ...'),
('quit', 'Syntax: .quit'),
('help', 'Syntax: .help [command]');

-- 3. Player Create Info (Crucial for boot)
-- Basic Start (Map 0, Northshire)
DELETE FROM `playercreateinfo`;
INSERT INTO `playercreateinfo` (`race`, `class`, `map`, `zone`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES 
(1, 1, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(1, 2, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(1, 3, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(1, 4, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(1, 5, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(1, 6, 609, 4298, 2355.84, -5663.33, 153.929, 3.65979), 
(1, 8, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(1, 9, 0, 12, -8949.95, -132.493, 83.5312, 0), 
(2, 1, 1, 14, -618.518, -4251.67, 38.2046, 0);
