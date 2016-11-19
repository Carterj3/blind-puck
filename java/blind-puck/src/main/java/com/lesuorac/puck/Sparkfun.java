package com.lesuorac.puck;

import java.util.List;

import org.springframework.cloud.netflix.feign.FeignClient;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

@FeignClient(value = "http://192.168.4.1")
public interface Sparkfun {

	@RequestMapping(path = "/adc", method = RequestMethod.GET)
	Integer getAdcVoltage();

	@RequestMapping(path = "/spiffOpen", method = RequestMethod.POST)
	SpiffOpenResponse configureSpiff(@RequestParam("path") String path,
			@RequestParam("mode") String mode,
			@RequestParam("content") String content);
}