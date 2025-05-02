import java.io.Serializable;
import java.util.Date;

public final class CalendarDate implements Serializable {
    private static final long serialVersionUID = 1L;
    private final Date date;
    private final String eventDescription;

    public CalendarDate(Date date, String eventDescription) {
        if (date == null || eventDescription == null) {
            throw new IllegalArgumentException("Null values not allowed");
        }
        if (eventDescription.length() > 1000) {
            throw new IllegalArgumentException("Description exceeds 1000 characters");
        }
        if (date.getTime() < System.currentTimeMillis() - 31536000000L) { // 1 year in past
            throw new IllegalArgumentException("Date cannot be older than 1 year");
        }
        if (date.getTime() > System.currentTimeMillis() + 31536000000L) { // 1 year in future
            throw new IllegalArgumentException("Date cannot be more than 1 year in future");
        }
        
        this.date = new Date(date.getTime());
        this.eventDescription = eventDescription;
    }

    public Date getDate() {
        return new Date(date.getTime());
    }

    public String getEventDescription() {
        return eventDescription;
    }

    @Override
    public String toString() {
        return "CalendarDate[date=" + date + ", description=" + 
               eventDescription.substring(0, Math.min(50, eventDescription.length())) + 
               (eventDescription.length() > 50 ? "..." : "") + "]";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        CalendarDate that = (CalendarDate) o;
        return date.equals(that.date) && 
               eventDescription.equals(that.eventDescription);
    }

    @Override
    public int hashCode() {
        return 31 * date.hashCode() + eventDescription.hashCode();
    }
}
