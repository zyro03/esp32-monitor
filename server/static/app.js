function updateClock() {
  const clock = document.getElementById("clock");

  if (clock) {
    clock.textContent = new Date().toLocaleTimeString("pl-PL");
  }
}

updateClock();
setInterval(updateClock, 1000);
