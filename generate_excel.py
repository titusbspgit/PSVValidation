#!/usr/bin/env python3
import json
import os
from datetime import datetime, timezone, timedelta

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
print(f"IST_TIMESTAMP={timestamp}")
