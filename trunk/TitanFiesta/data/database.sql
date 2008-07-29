-- phpMyAdmin SQL Dump
-- version 2.10.3
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Jul 28, 2008 at 01:36 AM
-- Server version: 5.0.45
-- PHP Version: 5.2.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

-- 
-- Database: `titanfiesta`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `users`
-- 

CREATE TABLE `users` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `username` varchar(18) collate latin1_general_ci NOT NULL,
  `password` varchar(32) collate latin1_general_ci NOT NULL,
  `accesslevel` tinyint(1) NOT NULL,
  `loginid` smallint(2) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci AUTO_INCREMENT=1 ;

-- 
-- Table structure for table `characters`
-- 

CREATE TABLE `characters` (
  `id` int(11) NOT NULL auto_increment,
  `owner` varchar(16) NOT NULL,
  `charname` varchar(16) NOT NULL,
  `map` varchar(12) NOT NULL default 'Rou',
  `money` bigint(20) NOT NULL default '0',
  `level` int(11) NOT NULL default '1',
  `maxhp` int(11) NOT NULL default '1',
  `curhp` int(11) NOT NULL default '1',
  `maxsp` int(11) NOT NULL default '1',
  `cursp` int(11) NOT NULL default '1',
  `exp` bigint(20) NOT NULL default '0',
  `fame` bigint(20) NOT NULL default '0',
  `profession` int(11) NOT NULL default '0',
  `ismale` tinyint(1) NOT NULL default '0',
  `hair` int(11) NOT NULL default '0',
  `haircolor` int(11) NOT NULL default '0',
  `face` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;