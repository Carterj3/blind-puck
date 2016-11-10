package com.lesuorac.puck;

import java.util.List;

import org.springframework.cloud.netflix.feign.FeignClient;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@FeignClient(value = "http://192.168.4.1")
public interface Sparkfun {
	
	@RequestMapping(path = "/", method = RequestMethod.GET)
	Integer getAdcVoltage();
}