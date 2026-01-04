USE `world`;

-- 1. Essential Trinity Strings
INSERT INTO `trinity_string` (`entry`, `content_default`) VALUES 
(1, 'TrinityCore/3.3.5a (Branch: 3.3.5)'),
(2, 'Using database: TDB 335.25101'),
(3, 'World initialized'),
(4, 'Account created: %s'),
(5, 'Welcome to the Blank MMO Admin Console.');

-- 2. Basic Commands
INSERT INTO `command` (`name`, `help`) VALUES 
('account', 'Syntax: .account ...'),
('account create', 'Syntax: .account create <name> <pass>'),
('account set', 'Syntax: .account set ...'),
('server', 'Syntax: .server ...'),
('quit', 'Syntax: .quit');

-- 3. Instance Template (Dummy)
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES (0, 0, '');
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES (1, 0, '');

-- 4. Worldstates
INSERT INTO `worldstates` (`entry`, `value`, `comment`) VALUES (20001, 1, 'Dummy Worldstate');