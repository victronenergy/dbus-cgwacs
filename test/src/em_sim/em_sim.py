import random
import time


class PowerInfo:
    def __init__(self):
        self._lastEnergy = 0
        self._lastEnergy_reverse = 0
        self._prevPower = 0
        # Use time.perf_counter() instead of time.time() when using python 3
        self._lastTimeStamp = time.time()

    @property
    def current(self):
        return random.gauss(14, 0.5)

    @property
    def voltage(self):
        return random.gauss(230, 0.05)

    @property
    def power(self):
        t = time.time()
        dt = t - self._lastTimeStamp
        p = random.gauss(1000, 10 * dt)
        dp = (self._prevPower + p) * dt / (2 * 3600)
        if dp > 0:
            self._lastEnergy += dp
        else:
            self._lastEnergy_reverse -= dp
        self._lastTimeStamp = t
        self._prevPower = p
        return p

    @property
    def energy(self):
        p = self.power
        return self._lastEnergy

    @property
    def energy_reverse(self):
        p = self.power
        return self._lastEnergy_reverse


class EmSim:
    def __init__(self, has_3phases=True):
        self.main = PowerInfo()
        self.has_3phases = has_3phases
        if has_3phases:
            self.l1 = PowerInfo()
            self.l2 = PowerInfo()
            self.l3 = PowerInfo()
